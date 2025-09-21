#pragma once
#include <vix/config.h>
#include <vix/types.h>

/* alignment for architectures that require it */
#define ARCH_ALIGNMENT_REQUIRED 8

/* needed for memory allocators -- VIRTUAL addresses -- must be aligned to CONFIG_ARCH_PAGE_SIZE */
#define ARCH_KERNEL_HEAP_START 0x190000
#define ARCH_KERNEL_HEAP_END   (ARCH_KERNEL_HEAP_START + 0xFFFFFF)
