#pragma once
#include <types.h>

namespace mm::phys {
    void phys_alloc(void *adr, size_t blockcount);
    void *phys_malloc(size_t blockcount);
    void phys_free(void *adr, size_t blockcount);
    size_t phys_get_free_blocks();
    void phys_init();
}
