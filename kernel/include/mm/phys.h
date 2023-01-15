#pragma once
#include <types.h>

namespace mm::phys {
    void phys_alloc(void *adr, size_t blockcount);
    void *phys_malloc(size_t blockcount);
    void phys_free(void *adr, size_t blockcount);
    size_t phys_get_free_blocks();
    void phys_init();

    /*
     * the mm::phys::kernel_* functions should ONLY be used by kmalloc!
     * the only exception is kernel_alloc in early initialisation code
     *
     * Why you may ask? because kmalloc wants to use all the rest of the memory for the kernel heap.
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
