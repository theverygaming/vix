#pragma once
#include <vix/config.h>
#include <vix/types.h>

/* alignment for architectures that require it */
#define ARCH_ALIGNMENT_REQUIRED 1

/* needed for memory allocators -- VIRTUAL addresses -- must be aligned to CONFIG_ARCH_PAGE_SIZE */

#ifdef CONFIG_ARCH_HAS_PAGING
#define ARCH_KERNEL_HEAP_START (CONFIG_KERNEL_HIGHER_HALF + CONFIG_HHDM_SIZE)
#define ARCH_KERNEL_HEAP_END   (ARCH_KERNEL_HEAP_START + 0x10000000) // ARCH_KERNEL_HEAP_START + 256MiB
#endif
