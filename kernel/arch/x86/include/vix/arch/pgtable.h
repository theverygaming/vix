#pragma once
#include <vix/config.h>
#include <vix/macros.h>
#include <vix/mm/mm.h>
#include <vix/panic.h>

#define PG_PDL1_SHIFT  22
#define PG_PDL1_NPTRS  1024
#define PG_PDL1_VAL(x) ((x).e)

#define PG_PT_SHIFT  12
#define PG_PT_NPTRS  1024
#define PG_PT_VAL(x) ((x).e)

namespace arch::pgtable {
    typedef struct {
        uint32_t e;
    } pdl4_t;

    typedef struct {
        uint32_t e;
    } pdl3_t;

    typedef struct {
        uint32_t e;
    } pdl2_t;

    typedef struct {
        uint32_t e;
    } pdl1_t;

    typedef struct {
        uint32_t e;
    } pt_t;
}

namespace arch::pgtable {
    static inline mm::vaddr_t hhdm_to_virt(mm::paddr_t addr) {
        if (unlikely(
                addr <= CONFIG_HHDM_PHYS_BASE ||
                addr >= (CONFIG_HHDM_PHYS_BASE + CONFIG_HHDM_SIZE)
            )) {
            KERNEL_PANIC("hhdm_to_virt: out of HHDM range: 0x%p", addr);
        }
        return (addr - CONFIG_HHDM_PHYS_BASE) + CONFIG_HHDM_VIRT_BASE;
    }

    static inline mm::paddr_t hhdm_to_phys(mm::vaddr_t addr) {
        if (unlikely(
                addr <= CONFIG_HHDM_VIRT_BASE ||
                addr >= (CONFIG_HHDM_VIRT_BASE + CONFIG_HHDM_SIZE)
            )) {
            KERNEL_PANIC("hhdm_to_phys: out of HHDM range: 0x%p", addr);
        }
        return (addr - CONFIG_HHDM_VIRT_BASE) + CONFIG_HHDM_PHYS_BASE;
    }

    static inline unsigned int
    x86_flags_get(uint32_t val, bool with_global_dirty) {
        unsigned int flags;
        if (val == 0) {
            return FLAG_NONE;
        }
        if (val & (1 << 0)) {
            flags |= FLAG_PRESENT;
        }
        if ((val & (1 << 1)) == 0) {
            flags |= FLAG_READ_ONLY;
        }
        if (val & (1 << 2)) {
            flags |= FLAG_USER;
        }
        if (val & (1 << 4)) {
            flags |= FLAG_CACHE_DISABLE;
        }
        if (val & (1 << 3)) {
            flags |= FLAG_WRITE_THROUGH;
        } else {
            flags |= FLAG_WRITE_BACK;
        }
        if (val & (1 << 5)) {
            flags |= FLAG_ACCESSED;
        }
        if (with_global_dirty && val & (1 << 6)) {
            flags |= FLAG_DIRTY;
        }
        if (with_global_dirty && val & (1 << 8)) {
            flags |= FLAG_NO_FLUSH_ON_PDL4_SET;
        }
        return flags;
    }

    static inline uint32_t
    x86_flags_set(unsigned int flags, bool with_global_dirty) {
        uint32_t val;
        if (flags & FLAG_NONE) {
            return 0;
        }
        if (flags & FLAG_PRESENT) {
            val |= (1 << 0);
        }
        if (!(flags & FLAG_READ_ONLY)) {
            val |= (1 << 1);
        }
        if (flags & FLAG_USER) {
            val |= (1 << 2);
        }
        if (flags & FLAG_CACHE_DISABLE) {
            val |= (1 << 4);
        }
        if (flags & FLAG_WRITE_THROUGH) {
            val |= (1 << 3);
        }
        if (flags & FLAG_ACCESSED) {
            val |= (1 << 5);
        }
        if (with_global_dirty && flags & FLAG_DIRTY) {
            val |= (1 << 6);
        }
        if (with_global_dirty && flags & FLAG_NO_FLUSH_ON_PDL4_SET) {
            val |= (1 << 8);
        }
        return val;
    }

    static inline unsigned int pt_flags_get(pt_t *pt) {
        uint32_t val = PG_PT_VAL(*pt);
        return x86_flags_get(val, true);
    }

    static inline void pt_flags_set(pt_t *pt, unsigned int flags) {
        uint32_t val = PG_PT_VAL(*pt) & ~0xFFF;
        val |= x86_flags_set(flags, true);
        PG_PT_VAL(*pt) = val;
    }

    static inline unsigned int pdl1_flags_get(pdl1_t *pdl1) {
        uint32_t val = PG_PDL1_VAL(*pdl1);
        unsigned int flags = 0;
        if (val & (1 << 7)) {
            flags |= FLAG_HUGEPAGE;
        }
        flags |= x86_flags_get(val, flags & FLAG_HUGEPAGE);
        return flags;
    }

    static inline void pdl1_flags_set(pdl1_t *pdl1, unsigned int flags) {
        uint32_t val = PG_PDL1_VAL(*pdl1);
        if (flags & FLAG_HUGEPAGE) {
            val &= ~0x1FFF;
            val |= (1 << 7);
        } else {
            val &= ~0xFFF;
        }
        val |= x86_flags_set(flags, flags & FLAG_HUGEPAGE);
        PG_PDL1_VAL(*pdl1) = val;
    }

#ifndef PG_pdl1_addr
    static inline mm::paddr_t pdl1_addr(pdl1_t *pdl1) {
        uint32_t val = PG_PDL1_VAL(*pdl1);
        // PS bit (4M page?)
        if (val & (1 << 7)) {
            return val & 0xFFE00000;
        }
        return val & 0xFFFFF000;
    }
#define PG_pdl1_addr
#endif

#ifndef PG_pt_addr
    static inline mm::paddr_t pt_addr(pt_t *pt) {
        return PG_PT_VAL(*pt) & 0xFFFFF000;
    }
#define PG_pt_addr
#endif
}

#define PGTABLE_FOLD_PDL4
#define PGTABLE_FOLD_PDL3
#define PGTABLE_FOLD_PDL2
#include <vix/arch-generic/pgtable_fold.h>
