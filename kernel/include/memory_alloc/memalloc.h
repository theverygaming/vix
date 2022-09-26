#pragma once
#include <types.h>

namespace memalloc::page {
    void phys_alloc(void *adr, uint32_t blockcount);
    void *phys_malloc(uint32_t blocks);
    void phys_free(void *adr);
    size_t phys_get_free_blocks();
    void phys_init();

    void *kernel_malloc(uint32_t blocks);
    void *kernel_realloc(void *adr, uint32_t blocks);
    void kernel_alloc(void *adr, uint32_t blocks);
    void kernel_free(void *adr);
    size_t kernel_get_free_blocks();
    void kernel_init();
}

namespace memalloc::single {
    void *kmalloc(size_t size);
    void kfree(void *ptr);
    void *krealloc(void *ptr, size_t size);
}
