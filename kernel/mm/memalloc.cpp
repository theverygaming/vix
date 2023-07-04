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
#include <panic.h>
#include <stdlib.h>

#define PHYS_BITMAP_BLOCK_COUNT ((ARCH_PHYS_MAX_MEM_ADR - ARCH_PHYS_MEM_START) / ARCH_PAGE_SIZE)

#define KERNEL_PAGES ((ARCH_KERNEL_HEAP_END / ARCH_PAGE_SIZE) - (ARCH_KERNEL_HEAP_START / ARCH_PAGE_SIZE))

static mm::allocators::block_alloc_single<PHYS_BITMAP_BLOCK_COUNT, ARCH_PAGE_SIZE> physalloc;

void mm::phys::phys_alloc(void *adr, size_t blockcount) {
    physalloc.alloc(((uint8_t *)adr) - ARCH_PHYS_MEM_START, blockcount);
}

void *mm::phys::phys_malloc(size_t blockcount) {
    bool success = false;
    uint8_t *allocated = (uint8_t *)physalloc.malloc(blockcount, &success);
    allocated += ARCH_PHYS_MEM_START;
    if (!success) {
        KERNEL_PANIC("phys_malloc -> memory full");
        return nullptr;
    }
    return allocated;
}

void mm::phys::phys_free(void *adr, size_t blockcount) {
    physalloc.dealloc(((uint8_t *)adr) - ARCH_PHYS_MEM_START, blockcount);
}

size_t mm::phys::phys_get_free_blocks() {
    return physalloc.count_free_blocks();
}

void mm::phys::phys_init() {
    physalloc.init();
    physalloc.markAllUsed();

    const struct mm::mem_map_entry *entry = nullptr;
    size_t counter = 0;

    // allocate all usable areas
    entry = mm::get_mem_map(counter);
    while (entry != nullptr) {
        if (entry->size != 0 && mm::memmap_is_usable(entry->type)) {
            uint64_t start = entry->base;
            uint64_t end = entry->base + entry->size;
            if (start < ARCH_PHYS_MAX_MEM_ADR && end > ARCH_PHYS_MEM_START) {
                start = std::max(start, (uint64_t)ARCH_PHYS_MEM_START);
                start = ALIGN_UP(start, ARCH_PAGE_SIZE);
                end = std::min(end, (uint64_t)ARCH_PHYS_MAX_MEM_ADR);
                end = ALIGN_DOWN(end, ARCH_PAGE_SIZE);
                physalloc.dealloc((uint8_t *)start - ARCH_PHYS_MEM_START, (end - start) / ARCH_PAGE_SIZE);
            }
        }
        counter++;
        entry = mm::get_mem_map(counter);
    }

    // now mark all areas that are unusable as allocated. Makes overlaps not be a problem
    counter = 0;
    entry = mm::get_mem_map(counter);
    while (entry != nullptr) {
        if ((entry->size != 0) && (!mm::memmap_is_usable(entry->type))) {
            uint64_t start = entry->base;
            uint64_t end = entry->base + entry->size;
            if (start < ARCH_PHYS_MAX_MEM_ADR && end > ARCH_PHYS_MEM_START) {
                start = std::max(start, (uint64_t)ARCH_PHYS_MEM_START);
                start = ALIGN_DOWN(start, ARCH_PAGE_SIZE);
                end = std::min(end, (uint64_t)ARCH_PHYS_MAX_MEM_ADR);
                end = ALIGN_UP(end, ARCH_PAGE_SIZE);
                physalloc.alloc((uint8_t *)start - ARCH_PHYS_MEM_START, (end - start) / ARCH_PAGE_SIZE);
            }
        }
        counter++;
        entry = mm::get_mem_map(counter);
    }

    kprintf(KP_INFO, "physmm: initialized physical memory manager\n");
    unsigned int freemem = (mm::phys::phys_get_free_blocks() * ARCH_PAGE_SIZE) / 1024;
    char unit = 'K';
    if (freemem >= 10000) {
        unit = 'M';
        freemem /= 1024;
    }
    kprintf(KP_INFO, "physmm: free physical memory: %u%ciB\n", freemem, unit);
}
