#include "vix/panic.h"
#include <vix/pgtable.h>
#include <vix/initcall.h>
#include <vix/status.h>
#include <vix/kprintf.h>

// status::StatusOr<arch::vmm::pte_t>
static void walk(
    mm::vaddr_t vaddr,
    bool create_pts,
    unsigned int create_pts_flags
) {
    if (create_pts) {
        KERNEL_PANIC("create_pts not implemented");
    }
    arch::pgtable::pdl4_t *pdl4 = arch::pgtable::pdl4_offset(vaddr);
    unsigned int pdl4_flags = pdl4_flags_get(pdl4);
    kprintf(KP_INFO, "pdl4 flags: 0x%p\n", pdl4_flags);
    if ((pdl4_flags & arch::pgtable::FLAG_PRESENT) == 0) {
        KERNEL_PANIC("skill issue");
    }
    kprintf(KP_INFO, "pdl4: 0x%p\n", pdl4);
    kprintf(KP_INFO, "*pdl4: 0x%p\n", (*pdl4).e);

    arch::pgtable::pdl3_t *pdl3 = arch::pgtable::pdl3_offset(pdl4, vaddr);
    unsigned int pdl3_flags = pdl3_flags_get(pdl3);
    kprintf(KP_INFO, "pdl3 flags: 0x%p\n", pdl3_flags);
    if ((pdl3_flags & arch::pgtable::FLAG_PRESENT) == 0) {
        KERNEL_PANIC("skill issue");
    }
    kprintf(KP_INFO, "pdl3: 0x%p\n", pdl3);
    kprintf(KP_INFO, "*pdl3: 0x%p\n", (*pdl3).e);

    arch::pgtable::pdl2_t *pdl2 = arch::pgtable::pdl2_offset(pdl3, vaddr);
    unsigned int pdl2_flags = pdl2_flags_get(pdl2);
    kprintf(KP_INFO, "pdl2 flags: 0x%p\n", pdl2_flags);
    if ((pdl2_flags & arch::pgtable::FLAG_PRESENT) == 0) {
        KERNEL_PANIC("skill issue");
    }
    kprintf(KP_INFO, "pdl2: 0x%p\n", pdl2);
    kprintf(KP_INFO, "*pdl2: 0x%p\n", (*pdl2).e);

    arch::pgtable::pdl1_t *pdl1 = arch::pgtable::pdl1_offset(pdl2, vaddr);
    unsigned int pdl1_flags = pdl1_flags_get(pdl1);
    kprintf(KP_INFO, "pdl1 flags: 0x%p\n", pdl1_flags);
    if ((pdl1_flags & arch::pgtable::FLAG_PRESENT) == 0) {
        KERNEL_PANIC("skill issue");
    }
    kprintf(KP_INFO, "pdl1: 0x%p\n", pdl1);
    kprintf(KP_INFO, "*pdl1: 0x%p\n", (*pdl1).e);

    arch::pgtable::pt_t *pt = arch::pgtable::pt_offset(pdl1, vaddr);
    unsigned int pt_flags = pt_flags_get(pt);
    kprintf(KP_INFO, "pt flags: 0x%p\n", pt_flags);
    if ((pt_flags & arch::pgtable::FLAG_PRESENT) == 0) {
        KERNEL_PANIC("skill issue");
    }
    kprintf(KP_INFO, "pt: 0x%p\n", pt);
    kprintf(KP_INFO, "*pt: 0x%p\n", (*pt).e);

    kprintf(KP_INFO, "pt: 0x%p\n", arch::pgtable::pt_addr(pt));
}

static int test() {
    walk(0xC0000000, false, 0);
    walk(0xC02af123, false, 0);
    //KERNEL_PANIC("good");
    return 0;
}

DEFINE_INITCALL(INITCALL_DRIVER_INIT, INITCALL_PRIO_NORMAL, test);
