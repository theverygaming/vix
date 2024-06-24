#include <algorithm>
#include <arch/generic/memory.h>
#include <config.h>
#include <debug.h>
#include <kprintf.h>
#include <macros.h>
#include <mm/allocators.h>
#include <mm/kmalloc.h>
#include <mm/memmap.h>
#include <mm/phys.h>
#include <mm/pmm.h>
#include <panic.h>
#include <stdlib.h>

#define PHYS_BITMAP_BLOCK_COUNT ((ARCH_PHYS_MAX_MEM_ADR - ARCH_PHYS_MEM_START) / ARCH_PAGE_SIZE)

#define KERNEL_PAGES ((ARCH_KERNEL_HEAP_END / ARCH_PAGE_SIZE) - (ARCH_KERNEL_HEAP_START / ARCH_PAGE_SIZE))

void mm::phys::phys_alloc(void *adr, size_t blockcount) {
    kprintf(KP_INFO, "falloc1: free blocks: %u\n", mm::pmm::get_free_blocks());
    mm::pmm::force_alloc_contiguous(adr, blockcount);
    kprintf(KP_INFO, "falloc2: free blocks: %u\n", mm::pmm::get_free_blocks());
}

void *mm::phys::phys_malloc(size_t blockcount) {
    kprintf(KP_INFO, "malloc1: asked for %u blocks\n", blockcount);
    kprintf(KP_INFO, "malloc1: free blocks: %u\n", mm::pmm::get_free_blocks());
    void *addr = mm::pmm::alloc_contiguous(blockcount);
    kprintf(KP_INFO, "malloc2: free blocks: %u .. 0x%p\n", mm::pmm::get_free_blocks(), addr);
    return addr;
}

void mm::phys::phys_free(void *adr, size_t blockcount) {
    kprintf(KP_INFO, "free1: free blocks: %u\n", mm::pmm::get_free_blocks());
    mm::pmm::free_contiguous(adr, blockcount);
    kprintf(KP_INFO, "free1: free blocks: %u\n", mm::pmm::get_free_blocks());
}

size_t mm::phys::phys_get_free_blocks() {
    return mm::pmm::get_free_blocks();
}

void mm::phys::phys_init() {
    mm::pmm::init();
    kprintf(KP_INFO, "free blocks: %u\n", mm::pmm::get_free_blocks());

    kprintf(KP_INFO, "physmm: initialized physical memory manager\n");
    unsigned int freemem = (mm::phys::phys_get_free_blocks() * ARCH_PAGE_SIZE) / 1024;
    char unit = 'K';
    if (freemem >= 10000) {
        unit = 'M';
        freemem /= 1024;
    }
    kprintf(KP_INFO, "physmm: free physical memory: %u%ciB\n", freemem, unit);
}
