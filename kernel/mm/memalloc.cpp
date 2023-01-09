#include <arch.h>
#include <config.h>
#include <log.h>
#include <mm/allocators.h>
#include <mm/memalloc.h>
#include <panic.h>
#include <stdlib.h>
#include INCLUDE_ARCH_GENERIC(memory.h)

#define PHYS_BITMAP_BLOCK_COUNT ((ARCH_PHYS_MAX_MEM_ADR - ARCH_PHYS_MEM_START) / ARCH_PAGE_SIZE)

#define KERNEL_PAGES ((ARCH_KERNEL_HEAP_END / ARCH_PAGE_SIZE) - (ARCH_KERNEL_HEAP_START / ARCH_PAGE_SIZE))

static memalloc::allocators::block_alloc<PHYS_BITMAP_BLOCK_COUNT, ARCH_PAGE_SIZE> physalloc;
static memalloc::allocators::block_alloc<KERNEL_PAGES, ARCH_PAGE_SIZE> kernelalloc;

void memalloc::page::phys_alloc(void *adr, uint32_t blockcount) {
    physalloc.alloc(((uint8_t *)adr) - ARCH_PHYS_MEM_START, blockcount);
}

void *memalloc::page::phys_malloc(uint32_t blockcount) {
    bool success = false;
    uint8_t *allocated = (uint8_t *)physalloc.malloc(blockcount, &success);
    allocated += ARCH_PHYS_MEM_START;
    if (!success) {
        KERNEL_PANIC("phys_malloc -> memory full");
        return nullptr;
    }
    return allocated;
}

void memalloc::page::phys_free(void *adr) {
    physalloc.free(((uint8_t *)adr) - ARCH_PHYS_MEM_START);
}

size_t memalloc::page::phys_get_free_blocks() {
    return physalloc.count_free_blocks();
}

void memalloc::page::phys_init() {
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
}

void *memalloc::page::kernel_malloc(uint32_t blockcount) {
    bool success = false;
    void *allocated = kernelalloc.malloc(blockcount, &success);
    if (!success) {
        KERNEL_PANIC("kernel_malloc -> memory full");
        return nullptr;
    }
    return ((uint8_t *)allocated) + ARCH_KERNEL_HEAP_START;
}

void memalloc::page::kernel_alloc(void *adr, uint32_t blockcount) {
    uint8_t *adr_p = (uint8_t *)adr;
    adr_p -= ARCH_KERNEL_HEAP_START;
    kernelalloc.alloc(adr_p, blockcount);
}

void *memalloc::page::kernel_realloc(void *adr, uint32_t blocks) {
    uint8_t *adr_p = (uint8_t *)adr;
    adr_p -= ARCH_KERNEL_HEAP_START;
    bool success;
    void *newptr = ((uint8_t *)kernelalloc.realloc(adr_p, blocks, &success)) + ARCH_KERNEL_HEAP_START;
    if (adr != newptr && success) {
        memcpy(newptr, adr, blocks * ARCH_PAGE_SIZE);
    }
    if (!success) {
        KERNEL_PANIC("kernel_realloc -> memory full");
        return nullptr;
    }
    return newptr;
}

void *memalloc::page::kernel_resize(void *adr, uint32_t blocks) {
    uint8_t *adr_p = (uint8_t *)adr;
    adr_p -= ARCH_KERNEL_HEAP_START;
    bool success;
    void *newptr = ((uint8_t *)kernelalloc.realloc(adr_p, blocks, &success)) + ARCH_KERNEL_HEAP_START;
    if (adr != newptr && success) {
        KERNEL_PANIC("unable to resize");
    }
    if (!success) {
        KERNEL_PANIC("kernel_resize -> memory full");
        return nullptr;
    }
    return newptr;
}

void memalloc::page::kernel_free(void *adr) {
    uint8_t *adr_p = (uint8_t *)adr;
    adr_p -= ARCH_KERNEL_HEAP_START;
    kernelalloc.free(adr_p);
}

size_t memalloc::page::kernel_get_free_blocks() {
    return kernelalloc.count_free_blocks();
}

void memalloc::page::kernel_init() {
    kernelalloc.init();
}
