#pragma once
#include <types.h>

namespace memalloc::page {
    void phys_alloc(void *adr, uint32_t blockcount);
    void *phys_malloc(uint32_t blocks);
    void phys_free(void *adr);
    size_t phys_get_free_blocks();
    void phys_init();

    /*
     * the memalloc::page::kernel_* functions should ONLY be used by memalloc::single!
     * the only exception is kernel_alloc in early initialisation code
     *
     * Why you may ask? because memalloc::single wants to use all the rest of the memory for the kernel heap.
     * Using one of these functions would result in kernel_resize most likely failing.
     *
     * TODO: in the future these should be removed entirely.
     */
    void *kernel_malloc(uint32_t blocks);
    void *kernel_realloc(void *adr, uint32_t blocks);
    void *kernel_resize(void *adr, uint32_t blocks);
    void kernel_alloc(void *adr, uint32_t blocks);
    void kernel_free(void *adr);
    size_t kernel_get_free_blocks();
    void kernel_init();
}

namespace mm {
    void *kmalloc(size_t size);
    void kfree(void *ptr);
    void *krealloc(void *ptr, size_t size);

    void *kmalloc_aligned(size_t size, size_t alignment); // impossible to free at the moment

    size_t getFreeSize();
    size_t getHeapFragmentation();
}
