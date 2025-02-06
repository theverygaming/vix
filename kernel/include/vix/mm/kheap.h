#pragma once
#include <vix/types.h>

namespace mm {
    void *kmalloc(size_t size);
    void kfree(void *ptr);
    void *krealloc(void *ptr, size_t size, size_t size_old);

    void *kmalloc_aligned(size_t size, size_t alignment); // impossible to free at the moment

    size_t getFreeSize();
    size_t getHeapFragmentation();

    // TODO: this should be moved to.. somewhere else idk where lmao
    // this is when you need physically contiguous memory, it will always return
    // memory aligned to the page size, do not use this to allocate significantly
    // less than the page size since that would be inefficient
    void *kmalloc_phys_contiguous(size_t size);
}
