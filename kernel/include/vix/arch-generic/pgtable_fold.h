#pragma once

#ifdef PGTABLE_FOLD_PDL1
#define PG_PDL1_SHIFT  PG_PT_SHIFT
#define PG_PDL1_NPTRS  1
#define PG_PDL1_VAL(x) PG_PT_VAL(x)
#endif

#ifdef PGTABLE_FOLD_PDL2
#define PG_PDL2_SHIFT  PG_PDL1_SHIFT
#define PG_PDL2_NPTRS  1
#define PG_PDL2_VAL(x) PG_PDL1_VAL(x)
#endif

#ifdef PGTABLE_FOLD_PDL3
#define PG_PDL3_SHIFT  PG_PDL2_SHIFT
#define PG_PDL3_NPTRS  1
#define PG_PDL3_VAL(x) PG_PDL2_VAL(x)
#endif

#ifdef PGTABLE_FOLD_PDL4
#define PG_PDL4_SHIFT  PG_PDL3_SHIFT
#define PG_PDL4_NPTRS  1
#define PG_PDL4_VAL(x) PG_PDL3_VAL(x)
#endif

namespace arch::pgtable {
#ifdef PGTABLE_FOLD_PDL1
#ifndef PG_pdl1_offset
    static inline pdl1_t *pdl1_offset(pdl2_t *pdl2, mm::vaddr_t addr) {
        return (pdl1_t *)pdl2;
    }
#define PG_pdl1_offset
#endif
#ifndef PG_pdl1_addr
    static inline mm::paddr_t pdl1_addr(pdl1_t *pdl1) {
        return hhdm_to_phys((mm::vaddr_t)pdl1);
    }
#define PG_pdl1_addr
#endif
    static inline unsigned int pdl1_flags_get(pdl1_t *pdl1) {
        unsigned int disallowed_flags = FLAG_HUGEPAGE | FLAG_NONE;
        unsigned int forced_flags = FLAG_PRESENT;
        return (pt_flags_get((pt_t *)pdl1) & ~disallowed_flags) | forced_flags;
    }
    static inline void pdl1_flags_set(pdl1_t *pdl1, unsigned int) {}
#endif

#ifdef PGTABLE_FOLD_PDL2
#ifndef PG_pdl2_offset
    static inline pdl2_t *pdl2_offset(pdl3_t *pdl3, mm::vaddr_t addr) {
        return (pdl2_t *)pdl3;
    }
#define PG_pdl2_offset
#endif
#ifndef PG_pdl2_addr
    static inline mm::paddr_t pdl2_addr(pdl2_t *pdl2) {
        return hhdm_to_phys((mm::vaddr_t)pdl2);
    }
#define PG_pdl2_addr
#endif
    static inline unsigned int pdl2_flags_get(pdl2_t *pdl2) {
        unsigned int disallowed_flags = FLAG_HUGEPAGE | FLAG_NONE;
        unsigned int forced_flags = FLAG_PRESENT;
        return (pdl1_flags_get((pdl1_t *)pdl2) & ~disallowed_flags) |
               forced_flags;
    }
    static inline void pdl2_flags_set(pdl2_t *pdl2, unsigned int) {}
#endif

#ifdef PGTABLE_FOLD_PDL3
#ifndef PG_pdl3_offset
    static inline pdl3_t *pdl3_offset(pdl4_t *pdl4, mm::vaddr_t addr) {
        return (pdl3_t *)pdl4;
    }
#define PG_pdl3_offset
#endif
#ifndef PG_pdl3_addr
    static inline mm::paddr_t pdl3_addr(pdl3_t *pdl3) {
        return hhdm_to_phys((mm::vaddr_t)pdl3);
    }
#define PG_pdl3_addr
#endif
    static inline unsigned int pdl3_flags_get(pdl3_t *pdl3) {
        unsigned int disallowed_flags = FLAG_HUGEPAGE | FLAG_NONE;
        unsigned int forced_flags = FLAG_PRESENT;
        return (pdl2_flags_get((pdl2_t *)pdl3) & ~disallowed_flags) |
               forced_flags;
    }
    static inline void pdl3_flags_set(pdl3_t *pdl3, unsigned int) {}
#endif

#ifdef PGTABLE_FOLD_PDL4
#ifndef PG_pdl4_addr
    static inline mm::paddr_t pdl4_addr(pdl4_t *pdl4) {
        return hhdm_to_phys((mm::vaddr_t)pdl4);
    }
#define PG_pdl4_addr
#endif
    static inline unsigned int pdl4_flags_get(pdl4_t *pdl4) {
        unsigned int disallowed_flags = FLAG_HUGEPAGE | FLAG_NONE;
        unsigned int forced_flags = FLAG_PRESENT;
        return (pdl3_flags_get((pdl3_t *)pdl4) & ~disallowed_flags) |
               forced_flags;
    }
    static inline void pdl4_flags_set(pdl4_t *pdl4, unsigned int) {}
#endif
}
