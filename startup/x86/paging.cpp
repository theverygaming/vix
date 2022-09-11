#include "paging.h"
#include "../../kernel/include/config.h"
#include <cmath>

uint32_t (*pagetables)[1024] = (uint32_t(*)[1024])(KERNEL_PHYS_ADDRESS + PAGE_TABLES_OFFSET);
uint32_t *page_directory = (uint32_t *)(KERNEL_PHYS_ADDRESS + PAGE_DIRECTORY_OFFSET);

extern "C" void loadPageDirectory(uint32_t *address);
extern "C" void enablePaging();

enum page_size { FOUR_KiB, FOUR_MB };
enum page_priv { SUPERVISOR, USER };
enum page_perms { R, RW };

struct directoryEntry {
    void *address;
    enum page_size pagesize;
    bool cache_disabled;
    bool write_through;
    enum page_priv priv;
    enum page_perms perms;
    bool present;
};

struct pagetableEntry {
    void *address;
    bool global;
    bool cache_disabled;
    bool write_through;
    enum page_priv priv;
    enum page_perms perms;
    bool present;
};

uint32_t make_directory_entry(struct directoryEntry de) {
    uint32_t entry = (uint32_t)de.address & 0xFFFFF000;
    entry |= de.pagesize << 7;
    entry |= de.cache_disabled << 4;
    entry |= de.write_through << 3;
    entry |= de.priv << 2;
    entry |= de.perms << 1;
    entry |= de.present;
    return entry;
}

uint32_t make_table_entry(struct pagetableEntry pe) {
    uint32_t entry = ((uint32_t)pe.address) & 0xFFFFF000;
    entry |= pe.global << 8;
    entry |= pe.cache_disabled << 6;
    entry |= pe.write_through << 3;
    entry |= pe.priv << 2;
    entry |= pe.perms << 1;
    entry |= pe.present;
    return entry;
}

void *paging::get_physaddr(void *virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    // TODO: check if entry is actually present
    return (void *)(pagetables[pdindex][ptindex] & 0xFFFFF000);
}

// Both addresses have to be page-aligned!
void map_page(void *physaddr, void *virtualaddr) {
    uint32_t pDirIndex = (uint32_t)virtualaddr >> 22;
    uint32_t pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;

    pagetables[pDirIndex][pTableIndex] = make_table_entry({.address = physaddr, .global = false, .cache_disabled = false, .write_through = false, .priv = SUPERVISOR, .perms = RW, .present = true});
}

void paging::initpaging() {
    for (int i = 0; i < 1024; i++) {
        for (unsigned int j = 0; j < 1024; j++) {
            pagetables[i][j] = 0;
        }
        page_directory[i] =
            make_directory_entry({.address = (void *)pagetables[i], .pagesize = FOUR_KiB, .cache_disabled = false, .write_through = false, .priv = SUPERVISOR, .perms = RW, .present = true});
    }

    for (int i = 0; i < 10000; i++) {
        map_page((void *)0 + (i * 0x1000), (void *)0 + (i * 0x1000));
    }
    
    for (int i = 0; i < (KERNEL_MEMORY_END_OFFSET / 4096); i++) {
        map_page((void *)KERNEL_PHYS_ADDRESS + (i * 0x1000), (void *)KERNEL_VIRT_ADDRESS + (i * 0x1000));
    }
    map_page((void *)0xB8000, (void *)(KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET)); // Video memory

    loadPageDirectory(page_directory);
    enablePaging();
    pagetables = (uint32_t(*)[1024])(KERNEL_VIRT_ADDRESS + PAGE_TABLES_OFFSET);
    page_directory = (uint32_t *)(KERNEL_VIRT_ADDRESS + PAGE_DIRECTORY_OFFSET);
}
