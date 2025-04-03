#pragma once
#include <vix/mm/mm.h>
#include <vix/status.h>
#include <vix/types.h>

namespace mm::pmm {
    void init();

    status::StatusOr<paddr_t> alloc_contiguous(size_t pages);
    void free_contiguous(paddr_t paddr, size_t pages);

    size_t get_free_blocks();

    void force_alloc_contiguous(paddr_t paddr, size_t pages);
    void force_free_contiguous(paddr_t paddr, size_t pages);
}
