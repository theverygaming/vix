#include "paging.h"
#include "stdio.h"
#include <cmath>
#include "../../config.h"


uint32_t (*pagetables)[1024] = (uint32_t(*)[1024])KERNEL_PHYS_ADDRESS + PAGE_TABLES_OFFSET;
uint32_t *page_directory = (uint32_t*)KERNEL_PHYS_ADDRESS + PAGE_DIRECTORY_OFFSET;

extern "C" void loadPageDirectory(uint32_t* address);
extern "C" void enablePaging();


void stage2_pagetablefill() {
    for(int i = 0; i < 6; i++) {
        for (unsigned int j = 0; j < 1024; j++)
        {
            paging::create_pagetable_entry(i, j, (void*)((j + (i * 1024)) * 0x1000), false, false, false, paging::SUPERVISOR, paging::RW, true);
        }
        paging::create_directory_entry(i, (void*)pagetables[i], paging::FOUR_KiB, 0, 0, paging::SUPERVISOR, paging::RW, true);
    }
}

void* paging::get_physaddr(void* virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
 
    // TODO: check if entry is actually present
    unsigned long *pd = (unsigned long *)page_directory;
 
    unsigned long *pt = ((unsigned long *)pagetables) + (0x400 * pdindex);
 
    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

uint32_t get_pd_index(void* virtualaddr) {
    float x = 0.1;
    floor(x);
}


void paging::initpaging()
{
    for (int i = 0; i < 1024; i++)
    {
        delete_directory_entry(i);
    }

    stage2_pagetablefill();
    printf("Allocated memory for stage2 ranges to: %dKB\n", (1024 * 6) * 4);
    printf("virt: %p, phys: %d\n", 0xC0000000, get_physaddr((void*)0xC0000000));
    get_pd_index((void*)0xC0000000);

    loadPageDirectory(page_directory);
    enablePaging();
}

void paging::create_pagetable_entry(int tablenum, int entrynum, void* address, bool global, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present) {
    uint32_t tentry = (uint32_t)address;
    tentry |= global << 8;
    tentry |= cache_disabled << 6;
    tentry |= write_through << 3;
    tentry |= priv << 2;
    tentry |= perms << 1;
    tentry |= present;
    pagetables[tablenum][entrynum] = tentry;
}

void paging::delete_pagetable_entry(int tablenum, int entrynum) {
    create_pagetable_entry(tablenum, entrynum, 0, false, true, false, SUPERVISOR, RW, false);
}

void paging::create_directory_entry(int tablenum, void* address, enum page_size pagesize, bool cache_disabled, bool write_through, enum page_priv priv, enum page_perms perms, bool present) {
    uint32_t direntry = (uint32_t)address;
    direntry |= pagesize << 7;
    direntry |= cache_disabled << 4;
    direntry |= write_through << 3;
    direntry |= priv << 2;
    direntry |= perms << 1;
    direntry |= present;
    page_directory[tablenum] = direntry;
}

void paging::delete_directory_entry(int tablenum) {
    create_directory_entry(tablenum, 0, FOUR_KiB, false, false, SUPERVISOR, RW, false);
}