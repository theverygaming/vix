#pragma once
#include <vix/config.h>

#ifdef CONFIG_ARCH_HAS_PAGING
#include <vix/mm/mm.h>
#include <vix/types.h>

namespace arch::pgtable {
    // if the page does not exist at all (not swapped out, not in RAM..)
    // NOTE: this flag is REQUIRED
    inline const unsigned int FLAG_NONE = (0x1 << 0);
    // This page does not point to any further down page table - instead
    // the walk must end here (set for e.g. 4MiB pages on IA-32)
    // NOT set for pages at the lowest level!
    // NOTE: this flag is REQUIRED
    inline const unsigned int FLAG_HUGEPAGE = (0x1 << 1);
    // if set the page is present in physical memory (not swapped out)
    // NOTE: this flag is REQUIRED
    inline const unsigned int FLAG_PRESENT = (0x1 << 2);
    // if set the page has been accessed (written, read etc.)
    inline const unsigned int FLAG_ACCESSED = (0x1 << 3);
    // if set the page has been written to
    inline const unsigned int FLAG_DIRTY = (0x1 << 4);

    // CACHE NOTE: these are exclusive, only one may be set
    // if set page will not be cached
    inline const unsigned int FLAG_CACHE_DISABLE = (0x1 << 5);
    // if set write-back caching will be enabled for this page
    inline const unsigned int FLAG_WRITE_BACK = (0x1 << 6);
    // if set write-through caching will be enabled for this page
    inline const unsigned int FLAG_WRITE_THROUGH = (0x1 << 7);
    // if set write-combined caching will be enabled for this page
    inline const unsigned int FLAG_WRITE_COMBINING = (0x1 << 8);

    // if set the page can be accessed from usermode
    inline const unsigned int FLAG_USER = (0x1 << 9);
    // if set the page will be read only to usermode and maybe even kernel mode
    inline const unsigned int FLAG_READ_ONLY = (0x1 << 10);
    // if set the page cannot be executed as code
    inline const unsigned int FLAG_NO_EXECUTE = (0x1 << 11);
    // if set the page will not be flushed when a new top-level page directory is set
    // (on x86 this is the global bit) -- this should never be enabled for user pages, bad idea, very bad
    inline const unsigned int FLAG_NO_FLUSH_ON_PDL4_SET = (0x1 << 12);
}

#include <vix/arch/pgtable.h>

// page directory level 4
#ifndef PG_PDL4_SIZE
#define PG_PDL4_SIZE (1UL << PG_PDL4_SHIFT)
#endif

// page directory level 3
#ifndef PG_PDL3_SIZE
#define PG_PDL3_SIZE (1UL << PG_PDL3_SHIFT)
#endif

// page directory level 2
#ifndef PG_PDL2_SIZE
#define PG_PDL2_SIZE (1UL << PG_PDL2_SHIFT)
#endif

// page directory level 1
#ifndef PG_PDL1_SIZE
#define PG_PDL1_SIZE (1UL << PG_PDL1_SHIFT)
#endif

// page table
#ifndef PG_PT_SIZE
#define PG_PT_SIZE (1UL << PG_PT_SHIFT)
#endif

namespace arch::pgtable {
#ifndef PG_pdl4_index
    static inline size_t pdl4_index(uintptr_t addr) {
        return (addr >> PG_PDL4_SHIFT) & (PG_PDL4_NPTRS - 1);
    }
#define PG_pdl4_index
#endif

#ifndef PG_pdl3_index
    static inline size_t pdl3_index(uintptr_t addr) {
        return (addr >> PG_PDL3_SHIFT) & (PG_PDL3_NPTRS - 1);
    }
#define PG_pdl3_index
#endif

#ifndef PG_pdl2_index
    static inline size_t pdl2_index(uintptr_t addr) {
        return (addr >> PG_PDL2_SHIFT) & (PG_PDL2_NPTRS - 1);
    }
#define PG_pdl2_index
#endif

#ifndef PG_pdl1_index
    static inline size_t pdl1_index(uintptr_t addr) {
        return (addr >> PG_PDL1_SHIFT) & (PG_PDL1_NPTRS - 1);
    }
#define PG_pdl1_index
#endif

#ifndef PG_pt_index
    static inline size_t pt_index(uintptr_t addr) {
        return (addr >> PG_PT_SHIFT) & (PG_PT_NPTRS - 1);
    }
#define PG_pt_index
#endif

    /*
        The kernel's global page directory.
        All pages for kernel memory are mapped here,
        the kernel part of this shall be copied to every new user page table
        // TODO: maybe at some point:tm: we'll need to store more
        //       information, then it might be a good idea to set
        //       up a struct instead that is per-process
    */
    extern pdl4_t *kernel_global_pdl4;

#ifndef PG_pdl4_offset
    // TODO: figure out the args, this needs some args esp for usermode stuff
    static inline pdl4_t *pdl4_offset(mm::vaddr_t addr) {
        return kernel_global_pdl4;
    }
#define PG_pdl4_offset
#endif

#ifndef PG_pdl3_offset
    static inline pdl3_t *pdl3_offset(pdl4_t *pdl4, uintptr_t addr) {
        return (pdl3_t *)hhdm_to_virt(pdl4_addr(pdl4)) + pdl3_index(addr);
    }
#define PG_pdl3_offset
#endif

#ifndef PG_pdl2_offset
    static inline pdl2_t *pdl2_offset(pdl3_t *pdl3, uintptr_t addr) {
        return (pdl2_t *)hhdm_to_virt(pdl3_addr(pdl3)) + pdl2_index(addr);
    }
#define PG_pdl2_offset
#endif

#ifndef PG_pdl1_offset
    static inline pdl1_t *pdl1_offset(pdl2_t *pdl2, uintptr_t addr) {
        return (pdl1_t *)hhdm_to_virt(pdl2_addr(pdl2)) + pdl1_index(addr);
    }
#define PG_pdl1_offset
#endif

#ifndef PG_pt_offset
    static inline pt_t *pt_offset(pdl1_t *pdl1, uintptr_t addr) {
        return (pt_t *)hhdm_to_virt(pdl1_addr(pdl1)) + pt_index(addr);
    }
#define PG_pt_offset
#endif
}

#endif
