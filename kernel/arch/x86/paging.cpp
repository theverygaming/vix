#include "paging.h"
#include "../config.h"
#include "stdio.h"
#include "stdlib.h"
#include <cmath>

uint32_t (*pagetables)[1024] = (uint32_t(*)[1024])(KERNEL_VIRT_ADDRESS + PAGE_TABLES_OFFSET);
uint32_t *page_directory = (uint32_t *)(KERNEL_VIRT_ADDRESS + PAGE_DIRECTORY_OFFSET);

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

struct directoryEntry get_directory_entry(uint32_t directoryEntry) {
    struct directoryEntry entry;
    entry.address = (void *)(directoryEntry & 0xFFFFF000);
    entry.present = directoryEntry & 0x1;
    entry.perms = (page_perms)((directoryEntry >> 1) & 0x1);
    entry.priv = (page_priv)((directoryEntry >> 2) & 0x1);
    entry.write_through = (directoryEntry >> 3) & 0x1;
    entry.cache_disabled = (directoryEntry >> 4) & 0x1;
    // entry.accessed = (directoryEntry >> 5) & 0x1;
    // entry.available = (directoryEntry >> 6) & 0x1;
    return entry;
}

struct pagetableEntry get_table_entry(uint32_t tableEntry) {
    struct pagetableEntry entry;
    entry.address = (void *)(tableEntry & 0xFFFFF000);
    entry.present = tableEntry & 0x1;
    entry.perms = (page_perms)((tableEntry >> 1) & 0x1);
    entry.priv = (page_priv)((tableEntry >> 2) & 0x1);
    entry.write_through = (tableEntry >> 3) & 0x1;
    entry.cache_disabled = (tableEntry >> 4) & 0x1;
    // entry.accessed = (tableEntry >> 5) & 0x1;
    // entry.dirty = (tableEntry >> 6) & 0x1;
    // entry.PAT = (tableEntry >> 7) & 0x1;
    entry.global = (tableEntry >> 8) & 0x1;
    // entry.available = (tableEntry >> 9) & 0x1;
    return entry;
}

void *paging::get_physaddr(void *virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    // TODO: check if entry is actually present
    return (void *)(pagetables[pdindex][ptindex] & 0xFFFFF000);
}

void invlpg(void *virtaddrx) {
    uint32_t virtaddr = (uint32_t)virtaddrx;
    asm volatile("invlpg (%0)" ::"r"(virtaddr) : "memory");
}

// Both addresses have to be page-aligned!
void paging::map_page(void *physaddr, void *virtualaddr) {
    uint32_t pDirIndex = (uint32_t)virtualaddr >> 22;
    uint32_t pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;

    bool do_invlpg = pagetables[pDirIndex][pTableIndex] & 0x1;
    pagetables[pDirIndex][pTableIndex] = make_table_entry({.address = physaddr, .global = false, .cache_disabled = false, .write_through = false, .priv = SUPERVISOR, .perms = RW, .present = true});
    if (do_invlpg) {
        invlpg(virtualaddr);
    }
}

void unmap_page(void *virtualaddr) {
    uint32_t pDirIndex = (uint32_t)virtualaddr >> 22;
    uint32_t pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;

    bool do_invlpg = pagetables[pDirIndex][pTableIndex] & 0x1;
    pagetables[pDirIndex][pTableIndex] = 0;
    if (do_invlpg) {
        invlpg(virtualaddr);
    }
}

void paging::clearPageTables(void *virtAddress, uint32_t pagecount) {
    for (uint32_t i = 0; i < pagecount; i++) {
        unmap_page(virtAddress + (i * 0x1000));
    }
}

bool paging::is_readable(const void *virtualaddr) {
    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;
    return pagetables[pdindex][ptindex] & 0x1;
}

void paging::copyPhysPage(void *dest, void *src) {
    uint32_t before1 = pagetables[0][0];
    uint32_t before2 = pagetables[0][1];
    pagetables[0][0] = make_table_entry({.address = src, .global = false, .cache_disabled = false, .write_through = false, .priv = SUPERVISOR, .perms = RW, .present = true});
    pagetables[0][1] = make_table_entry({.address = dest, .global = false, .cache_disabled = false, .write_through = false, .priv = SUPERVISOR, .perms = RW, .present = true});
    invlpg((void *)0);
    invlpg((void *)4096);
    memcpy((char *)4096, (char *)0, 4096);
    pagetables[0][0] = before1;
    pagetables[0][1] = before2;
    invlpg((void *)0);
    invlpg((void *)4096);
}
