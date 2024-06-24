#pragma once
#include <types.h>

namespace mm::pmm {
    void init();

    void *alloc_contiguous(size_t pages);
    void free_contiguous(void *paddr, size_t pages);

    size_t get_free_blocks();

    void force_alloc_contiguous(void *paddr, size_t pages);
    void force_free_contiguous(void *paddr, size_t pages);
}
