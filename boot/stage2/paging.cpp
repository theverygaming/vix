#include "paging.h"
#include "stdio.h"
#include <cmath>
#include "../../config.h"


uint32_t (*pagetables)[1024] = (uint32_t(*)[1024])(KERNEL_PHYS_ADDRESS + PAGE_TABLES_OFFSET);
uint32_t *page_directory = (uint32_t*)(KERNEL_PHYS_ADDRESS + PAGE_DIRECTORY_OFFSET);

extern "C" void loadPageDirectory(uint32_t* address);
extern "C" void enablePaging();


void* paging::get_physaddr(void* virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
 
    // TODO: check if entry is actually present
    unsigned long *pd = (unsigned long *)page_directory;
 
    unsigned long *pt = ((unsigned long *)pagetables) + (0x400 * pdindex);
 
    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

//Both addresses have to be page-aligned!
void map_page(void *physaddr, void *virtualaddr) {
    unsigned long pDirIndex = (unsigned long)virtualaddr >> 22;
    unsigned long pTableIndex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    paging::create_pagetable_entry(pDirIndex, pTableIndex, physaddr, false, false, false, paging::SUPERVISOR, paging::RW, true);
    paging::create_directory_entry(pDirIndex, (void*)pagetables[pDirIndex], paging::FOUR_KiB, false, false, paging::SUPERVISOR, paging::RW, true);
}

void stage2_pagetablefill() {
    for(int i = 0; i < 10000; i++) {
        map_page((void*)0 + (i * 0x1000), (void*)0 + (i * 0x1000));
    }
}

void paging::initpaging()
{
    for (int i = 0; i < 1024; i++)
    {
        for (unsigned int j = 0; j < 1024; j++)
        {
            create_pagetable_entry(i, j, (void*)0, false, false, false, SUPERVISOR, RW, false);
        }
        paging::create_directory_entry(i, (void*)pagetables[i], paging::FOUR_KiB, false, false, paging::SUPERVISOR, paging::RW, true);
    }

    stage2_pagetablefill();
    for(int i = 0; i < (KERNEL_MEMORY_END_OFFSET / 4096); i++) {
        map_page((void*)KERNEL_PHYS_ADDRESS + (i * 0x1000), (void*)KERNEL_VIRT_ADDRESS + (i * 0x1000));
    }
    map_page((void*)0xB8000, (void*)(KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET)); // Video memory
    
    loadPageDirectory(page_directory);
    enablePaging();
    pagetables = (uint32_t(*)[1024])(KERNEL_VIRT_ADDRESS + PAGE_TABLES_OFFSET);
    page_directory = (uint32_t*)(KERNEL_VIRT_ADDRESS + PAGE_DIRECTORY_OFFSET);
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