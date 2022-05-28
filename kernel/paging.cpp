#include "paging.h"
#include "stdio.h"


//uint32_t pagetables[1024][1024] __attribute__((aligned(4096)));
//uint32_t page_directory[1024] __attribute__((aligned(4096)));

uint32_t (*pagetables)[1024] = (uint32_t(*)[1024])0x1002000; //size: 4194304 bytes
uint32_t *page_directory = (uint32_t*)0x1001000; // size: 4096 bytes

extern "C" void loadPageDirectory(uint32_t* address);
extern "C" void enablePaging();


void kernel_pagetablefill(int tablenum, bool global, bool cache_disabled, bool write_through, enum paging::page_priv priv, enum paging::page_perms perms, bool present) {
    for (unsigned int i = 0; i < 1024; i++)
    {
        paging::create_pagetable_entry(tablenum, i, (void*)((i + (tablenum * 1024)) * 0x1000), global, cache_disabled, write_through, priv, perms, present);
    }
}


void paging::initpaging()
{
    for (int i = 0; i < 1024; i++)
    {
        delete_directory_entry(i);
    }

    for(int i = 0; i < 6; i++) {
        kernel_pagetablefill(i, false, false, false, SUPERVISOR, RW, true);
        create_directory_entry(i, (void*)pagetables[i], FOUR_KB, 0, 0, SUPERVISOR, RW, 1);
    }
    printf("Allocated memory for kernel ranges to: %dKB\n", (1024 * 6) * 4);

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
    create_directory_entry(tablenum, 0, FOUR_KB, false, false, SUPERVISOR, RW, false);
}