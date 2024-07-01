#pragma once
#include <types.h>

namespace mm {
    void *kmalloc(size_t size);
    void kfree(void *ptr);
    void *krealloc(void *ptr, size_t size);

    void *kmalloc_aligned(size_t size, size_t alignment); // impossible to free at the moment

    size_t getFreeSize();
    size_t getHeapFragmentation();
}
