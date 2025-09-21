#pragma once
#include <vix/arch/generic/archspecific.h>
#include <vix/types.h>

/* alignment for architectures that require it */
#define ARCH_ALIGNMENT_REQUIRED 8

/* needed for memory allocators -- VIRTUAL addresses -- must be aligned to CONFIG_ARCH_PAGE_SIZE */
#define ARCH_KERNEL_HEAP_START (KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET)
#define ARCH_KERNEL_HEAP_END   (KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)
