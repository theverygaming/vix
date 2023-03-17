#include <arch/generic/memory.h>
#include <config.h>
#include <debug.h>
#include <kprintf.h>
#include <mm/allocators.h>
#include <mm/kmalloc.h>
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

    arch::generic::memory::memory_map_entry entry;
    int counter = 0;

    // allocate all usable areas
    while (arch::generic::memory::get_memory_map(&entry, counter)) {
        size_t end_adr = entry.start_address + entry.size;

        if (entry.entry_type == arch::generic::memory::memory_map_entry::entry_type::MEMORY_RAM) {
            size_t new_start_address = entry.start_address;
            if (new_start_address < ARCH_PHYS_MAX_MEM_ADR) {
                if (new_start_address < ARCH_PHYS_MEM_START) {
                    new_start_address = ARCH_PHYS_MEM_START;
                }
                physalloc.dealloc((uint8_t *)new_start_address - ARCH_PHYS_MEM_START, (end_adr - entry.start_address) / ARCH_PAGE_SIZE);
            }
        }
        counter++;
    }

    // now mark all areas that are unusable as allocated. This has to be done to make sure that higher priority entries come first in case of overlaps
    counter = 0;
    while (arch::generic::memory::get_memory_map(&entry, counter)) {
        size_t end_adr = entry.start_address + entry.size;

        if (entry.entry_type != arch::generic::memory::memory_map_entry::entry_type::MEMORY_RAM) {
            size_t new_start_address = entry.start_address;
            if (new_start_address < ARCH_PHYS_MAX_MEM_ADR) {
                if (new_start_address < ARCH_PHYS_MEM_START) {
                    new_start_address = ARCH_PHYS_MEM_START;
                }
                physalloc.alloc((uint8_t *)entry.start_address - ARCH_PHYS_MEM_START, (end_adr - entry.start_address) / ARCH_PAGE_SIZE);
            }
        }
        counter++;
    }

    kprintf(KP_INFO, "physmm: initialized physical memory manager\n");
}
