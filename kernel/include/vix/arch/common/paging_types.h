#pragma once
// This file exists in case an architecture wants to override these types

#include <vix/config.h>
#include <vix/mm/mm.h>
#include <vix/types.h>

#ifdef CONFIG_ARCH_HAS_PAGING

#define PT_T_LEVEL(pt) ((pt).level)

namespace arch::vmm {
    // page table
    typedef struct {
        short level;
        mm::paddr_t ptr;
    } pt_t;

    // page table entry
    typedef uintptr_t pte_t;
}
#endif
