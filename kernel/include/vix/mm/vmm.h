#pragma once
#include <vix/config.h>
#include <vix/mm/mm.h>
#include <vix/types.h>

#ifdef CONFIG_ARCH_HAS_PAGING
namespace mm::vmm {
    void init();
    // FIXME: this shit aint thread safe at all :sob:
    vaddr_t find_free(vaddr_range range, size_t pages);
}
#endif
