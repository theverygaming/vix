#pragma once
#include <config.h>
#include <types.h>

#ifdef CONFIG_ARCH_HAS_PAGING
namespace mm::kv {
    void init();
    void *alloc(size_t pages);
    void free(void *address, size_t pages);
}
#endif
