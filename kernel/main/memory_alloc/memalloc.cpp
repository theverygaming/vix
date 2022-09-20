#include <arch/arch.h>
#include <config.h>
#include <log.h>
#include <memory_alloc/allocators.h>
#include <memory_alloc/memalloc.h>
#include <panic.h>
#include <stdlib.h>
#include INCLUDE_ARCH_GENERIC(memory.h)

#define PHYS_BITMAP_BLOCK_COUNT ARCH_PHYS_MAX_MEM / ARCH_PAGE_SIZE

#define KERNEL_PAGES ((ARCH_KERNEL_HEAP_END / ARCH_PAGE_SIZE) - (ARCH_KERNEL_HEAP_START / ARCH_PAGE_SIZE))

static memalloc::allocators::block_alloc<PHYS_BITMAP_BLOCK_COUNT, ARCH_PAGE_SIZE> physalloc;
static memalloc::allocators::block_alloc<KERNEL_PAGES, ARCH_PAGE_SIZE> kernelalloc;

void *memalloc::page::phys_malloc(uint32_t blockcount) {
    bool success = false;
    void *allocated = physalloc.malloc(blockcount, &success);
    if (!success) {
        KERNEL_PANIC("phys_malloc -> memory full");
        return nullptr;
    }
    return allocated;
}

void memalloc::page::phys_free(void *adr) {
    physalloc.free(adr);
}

void memalloc::page::phys_init() {
    physalloc.init();
    physalloc.markAllUsed();

    arch::generic::memory::memory_map_entry entry;
    int counter = 0;

    // allocate all usable areas
    while (arch::generic::memory::get_memory_map(&entry, counter)) {
        size_t end_adr = entry.start_address + entry.size;
        if ((end_adr / ARCH_PAGE_SIZE) > PHYS_BITMAP_BLOCK_COUNT) {
            end_adr = (PHYS_BITMAP_BLOCK_COUNT)*ARCH_PAGE_SIZE;
        }
        if ((entry.start_address / ARCH_PAGE_SIZE) > PHYS_BITMAP_BLOCK_COUNT) {
            LOG_DEBUG("entry bigger than memory map");
            counter++;
            continue;
        }

        if (entry.entry_type == arch::generic::memory::memory_map_entry::entry_type::MEMORY_RAM) {
            physalloc.dealloc((void *)entry.start_address, (end_adr - entry.start_address) / ARCH_PAGE_SIZE);
        }
        counter++;
    }

    // now mark all areas that are unusable as allocated. This has to be done to make sure that higher priority entires come first in case of overlaps
    counter = 0;
    while (arch::generic::memory::get_memory_map(&entry, counter)) {
        size_t end_adr = entry.start_address + entry.size;
        if ((end_adr / ARCH_PAGE_SIZE) > PHYS_BITMAP_BLOCK_COUNT) {
            end_adr = (PHYS_BITMAP_BLOCK_COUNT)*ARCH_PAGE_SIZE;
        }
        if ((entry.start_address / ARCH_PAGE_SIZE) > PHYS_BITMAP_BLOCK_COUNT) {
            LOG_DEBUG("entry bigger than memory map");
            counter++;
            continue;
        }

        if (entry.entry_type != arch::generic::memory::memory_map_entry::entry_type::MEMORY_RAM) {
            physalloc.alloc((void *)entry.start_address, (end_adr - entry.start_address) / ARCH_PAGE_SIZE);
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
    return (void *)(allocated + ARCH_KERNEL_HEAP_START);
}

void memalloc::page::kernel_alloc(void *adr, uint32_t blockcount) {
    adr -= ARCH_KERNEL_HEAP_START;
    kernelalloc.alloc(adr, blockcount);
}

void *memalloc::page::kernel_realloc(void *adr, uint32_t blocks) {
    void *oldptr = adr;
    adr -= ARCH_KERNEL_HEAP_START;
    bool success;
    void *newptr = kernelalloc.realloc(adr, blocks, &success) + ARCH_KERNEL_HEAP_START;
    if (oldptr != newptr && success) {
        memcpy((char *)newptr, (char *)oldptr, blocks * ARCH_PAGE_SIZE);
    }
    if (!success) {
        return nullptr;
    }
    return newptr;
}

void memalloc::page::kernel_free(void *adr) {
    adr -= ARCH_KERNEL_HEAP_START;
    kernelalloc.free(adr);
}

void memalloc::page::kernel_init() {
    kernelalloc.init();
}
