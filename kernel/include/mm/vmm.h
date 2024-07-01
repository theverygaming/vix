#pragma once
#include <config.h>
#include <types.h>

#ifdef CONFIG_ARCH_HAS_PAGING
namespace mm::vmm {
    void init();
    void *alloc(void *start, void *end, size_t pages);
    void *kalloc(size_t pages);
    void dealloc(void *start, size_t pages);
}
#endif
