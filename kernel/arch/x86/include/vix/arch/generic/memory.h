#pragma once
#include <vix/arch/generic/archspecific.h>
#include <vix/types.h>

/* alignment for architectures that require it */
#define ARCH_ALIGNMENT_REQUIRED 1

/* needed for memory allocators -- VIRTUAL addresses -- must be aligned to CONFIG_ARCH_PAGE_SIZE */
#define ARCH_KERNEL_HEAP_START (KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET)
#define ARCH_KERNEL_HEAP_END   (KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)

/* start address of physical memory, used by physical memory allocator */
#define ARCH_PHYS_MEM_START 0
/*
 * abosolute maximum PHYSICAL memory address this architecture can possibly have
 * this is used(together with CONFIG_ARCH_PAGE_SIZE) for example by kernel/main/memory_alloc/memalloc.cpp(the page allocator) for it's bitmap
 */
#define ARCH_PHYS_MAX_MEM_ADR 0xFFFFFFFF
