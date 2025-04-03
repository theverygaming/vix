#pragma once
#include <vix/config.h>
#include <vix/mm/mm.h>
#include <vix/types.h>

#ifdef CONFIG_ARCH_HAS_PAGING
namespace mm::vmm {
    void init();
    // TODO: the names for these functions are honestly quite bad! alloc and kalloc don't do anything!
    // FIXME: also this shit aint thread safe at all :sob:
    vaddr_t alloc(vaddr_t start, vaddr_t end, size_t pages);
    vaddr_t kalloc(size_t pages);
    void dealloc(vaddr_t start, size_t pages);
}
#endif
