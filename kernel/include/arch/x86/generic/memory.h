#pragma once
#include <arch/x86/generic/archspecific.h>
#include <types.h>

/* needed for memory allocators -- VIRTUAL addresses -- must be aligned to ARCH_PAGE_SIZE */
#define ARCH_KERNEL_HEAP_START (KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET)
#define ARCH_KERNEL_HEAP_END (KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)

/*
 * page size(if this architecture has virtual memory), otherwise just a block size used for some memory allocators and stuff
 */
#define ARCH_PAGE_SIZE 4096

/* start address of physical memory, used by physical memory allocator */
#define ARCH_PHYS_MEM_START 0
/*
 * abosolute maximum PHYSICAL memory address this architecture can possibly have
 * this is used(together with ARCH_PAGE_SIZE) for example by kernel/main/memory_alloc/memalloc.cpp(the page allocator) for it's bitmap
 */
#define ARCH_PHYS_MAX_MEM_ADR 0xFFFFFFFF

namespace arch::generic::memory {
    /* memory map entry struct */
    struct memory_map_entry {
        size_t start_address;
        size_t size;
        /* higher priority entries come first, possibly overwriting lower priority ones. Entry priority(lowest first) is the order of elements in this enum */
        enum class entry_type {
            MEMORY_UNUSABLE,
            MEMORY_ROM,
            MEMORY_MIMO,   /* memory mapped devices */
            MEMORY_KERNEL, /* where the kernel code, bss etc. sits */
            MEMORY_RAM,    /* this is the ONLY memory type that may be accessed without prior precautions */
        } entry_type;
    };
    /* call for each memory map entry, returns false when no more entires left -> n is a counter that starts with 0 */
    bool get_memory_map(struct memory_map_entry *entry, int n);
}
