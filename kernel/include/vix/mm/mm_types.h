#pragma once
#include <vix/types.h>
#include <vix/config.h>

namespace mm {
    // physical address
    typedef uintptr_t paddr_t;
    // virtual address
    typedef uintptr_t vaddr_t;
    // application address - application-facing memory (either it's a virtual address if the architecture has a MMU or a physical one if it doesn't)
#ifdef CONFIG_ARCH_HAS_PAGING
    typedef vaddr_t aaddr_t;
#else
    typedef paddr_t aaddr_t;
#endif

    inline const vaddr_t PADDR_MAX = UINTPTR_MAX;
    inline const vaddr_t VADDR_MAX = UINTPTR_MAX;
#ifdef CONFIG_ARCH_HAS_PAGING
    inline const aaddr_t AADDR_MAX = VADDR_MAX;
#else
    inline const aaddr_t AADDR_MAX = PADDR_MAX;
#endif

    // physical memory range
    struct paddr_range {
        paddr_t start;
        paddr_t end;
    };

    // virtual memory range
    struct vaddr_range {
        vaddr_t start;
        vaddr_t end;
    };

    // application memory range
    struct aaddr_range {
        aaddr_t start;
        aaddr_t end;
    };
}
