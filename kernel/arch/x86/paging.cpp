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

void create_directory_entry(int tablenum, void *address, enum page_size pagesize, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present) {
    uint32_t direntry = (uint32_t)address;
    direntry |= pagesize << 7;
    direntry |= cache_disabled << 4;
    direntry |= write_through << 3;
    direntry |= priv << 2;
    direntry |= perms << 1;
    direntry |= present;
    page_directory[tablenum] = direntry;
}

void set_pagetable_entry(int tablenum, int entrynum, void *address, bool global, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present) {
    uint32_t tentry = pagetables[tablenum][entrynum];
    tentry = (tentry & ~0xFFFFF000) | (((uint32_t)address) & 0xFFFFF000);
    tentry = (tentry & ~0xFFF) | (0 & 0xFFF);
    tentry |= global << 8;
    tentry |= cache_disabled << 6;
    tentry |= write_through << 3;
    tentry |= priv << 2;
    tentry |= perms << 1;
    tentry |= present;
    pagetables[tablenum][entrynum] = tentry;
}

void create_pagetable_entry(int tablenum, int entrynum, void *address, bool global, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present) {
    uint32_t tentry = (uint32_t)address;
    tentry |= global << 8;
    tentry |= cache_disabled << 6;
    tentry |= write_through << 3;
    tentry |= priv << 2;
    tentry |= perms << 1;
    tentry |= present;
    pagetables[tablenum][entrynum] = tentry;
}

void set_directory_entry(int tablenum, void *address, enum page_size pagesize, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present) {
    uint32_t direntry = page_directory[tablenum];
    direntry = (direntry & ~0xFFFFF000) | (((uint32_t)address) & 0xFFFFF000);
    direntry = (direntry & ~0xFFF) | (0 & 0xFFF);
    direntry |= pagesize << 7;
    direntry |= cache_disabled << 4;
    direntry |= write_through << 3;
    direntry |= priv << 2;
    direntry |= perms << 1;
    direntry |= present;
    page_directory[tablenum] = direntry;
}

void delete_pagetable_entry(int tablenum, int entrynum) {
    set_pagetable_entry(tablenum, entrynum, 0, false, true, false, SUPERVISOR, RW, false);
}

bool check_directory_entry_present(int tablenum) {
    return page_directory[tablenum] & 0x1;
}

bool check_pagetable_entry_present(int tablenum, int entrynum) {
    return pagetables[tablenum][entrynum] & 0x1;
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
    // printf("invlpg 0x%p\n", virtaddr);
}

// Both addresses have to be page-aligned!
void paging::map_page(void *physaddr, void *virtualaddr) {
    uint32_t pDirIndex = (uint32_t)virtualaddr >> 22;
    uint32_t pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;

    bool do_invlpg = check_pagetable_entry_present(pDirIndex, pTableIndex);
    set_pagetable_entry(pDirIndex, pTableIndex, physaddr, false, false, false, SUPERVISOR, RW, true);
    if (do_invlpg) {
        invlpg(virtualaddr);
    }
}

void unmap_page(void *virtualaddr) {
    uint32_t pDirIndex = (uint32_t)virtualaddr >> 22;
    uint32_t pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;

    bool do_invlpg = check_pagetable_entry_present(pDirIndex, pTableIndex);
    set_pagetable_entry(pDirIndex, pTableIndex, (void *)0, false, false, false, SUPERVISOR, RW, false);
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
    return check_pagetable_entry_present(pdindex, ptindex);
}

void paging::copyPhysPage(void *dest, void *src) {
    uint32_t before1 = pagetables[0][0];
    uint32_t before2 = pagetables[0][1];
    set_pagetable_entry(0, 0, src, false, false, false, SUPERVISOR, RW, true);
    set_pagetable_entry(0, 1, dest, false, false, false, SUPERVISOR, RW, true);
    invlpg((void *)0);
    invlpg((void *)4096);
    memcpy((char *)4096, (char *)0, 4096);
    pagetables[0][0] = before1;
    pagetables[0][1] = before2;
    invlpg((void *)0);
    invlpg((void *)4096);
}
