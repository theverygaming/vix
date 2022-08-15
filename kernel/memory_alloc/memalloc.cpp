#include "memalloc.h"
#include "../config.h"
#include "stdio.h"
#include "stdlib.h"
#include <memory_alloc/allocators.h>

#define PAGE_SIZE 4096

#define PHYS_MAX_MEM 4294963200

#define PHYS_BITMAP_BLOCK_COUNT PHYS_MAX_MEM / PAGE_SIZE

#define KERNEL_PAGES ((KERNEL_MEMORY_END_OFFSET / PAGE_SIZE) - (KERNEL_FREE_AREA_BEGIN_OFFSET / PAGE_SIZE))
#define KERNEL_ALL_PAGES (KERNEL_MEMORY_END_OFFSET / PAGE_SIZE)

static memalloc::allocators::block_alloc<PHYS_BITMAP_BLOCK_COUNT, PAGE_SIZE> physalloc;
static memalloc::allocators::block_alloc<KERNEL_PAGES, PAGE_SIZE> kernelalloc;

void *memalloc::page::phys_malloc(uint32_t blockcount) {
    bool success = false;
    void *allocated = physalloc.malloc(blockcount, &success);
    if (!success) {
        printf("memalloc::page::phys_malloc: memory full\n");
        return nullptr;
    }
    return allocated;
}

void memalloc::page::phys_free(void *adr) {
    physalloc.free(adr);
}

void memalloc::page::phys_init(memorymap::SMAP_entry *e620_map, int e620_len) {
    physalloc.init();
    physalloc.markAllUsed();
    // first set all the areas marked as usable
    for (int i = 0; i < e620_len; i++) {
        uint64_t start = e620_map[i].Base;
        uint64_t end = (e620_map[i].Base + e620_map[i].Length);
        if (end / PAGE_SIZE > PHYS_BITMAP_BLOCK_COUNT) {
            end = (PHYS_BITMAP_BLOCK_COUNT - 1) * PAGE_SIZE;
        }
        if (start / PAGE_SIZE > PHYS_BITMAP_BLOCK_COUNT) {
            printf("memalloc::page::phys_init: entry bigger than memory map\n");
            continue;
        }
        if (e620_map[i].Type == 1) {
            physalloc.dealloc((void *)start, (end - start) / PAGE_SIZE);
        }
    }
    // now set all areas marked as unusable
    for (int i = 0; i < e620_len; i++) {
        uint64_t start = e620_map[i].Base;
        uint64_t end = (e620_map[i].Base + e620_map[i].Length);
        if (end / PAGE_SIZE > PHYS_BITMAP_BLOCK_COUNT) {
            end = (PHYS_BITMAP_BLOCK_COUNT - 1) * PAGE_SIZE;
        }
        if (start / PAGE_SIZE > PHYS_BITMAP_BLOCK_COUNT) {
            printf("memalloc::page::phys_init: entry bigger than memory map\n");
            continue;
        }
        if (e620_map[i].Type > 1) {
            physalloc.alloc((void *)start, (end - start) / PAGE_SIZE);
        }
    }

    // mark all physical memory used by the kernel and bootloader as used
    // TODO: can we just free the bootloader memory?
    physalloc.alloc(nullptr, KERNEL_ALL_PAGES + (KERNEL_PHYS_ADDRESS / PAGE_SIZE));
}

void *memalloc::page::kernel_malloc(uint32_t blockcount) {
    bool success = false;
    void *allocated = kernelalloc.malloc(blockcount, &success);
    if (!success) {
        printf("memalloc::page::kernel_malloc memory full\n");
        return nullptr;
    }
    return (void *)(allocated + KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET);
}

void memalloc::page::kernel_alloc(void *adr, uint32_t blockcount) {
    adr -= KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET;
    kernelalloc.alloc(adr, blockcount);
}

void memalloc::page::kernel_free(void *adr) {
    adr -= KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET;
    kernelalloc.free(adr);
}

void memalloc::page::kernel_init() {
    kernelalloc.init();
}