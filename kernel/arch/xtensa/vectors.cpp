#include "vix/panic.h"
#include <vix/arch/vectors.h>
#include <vix/kprintf.h>

extern "C" void xtensa_setup_vectors();

void arch::init_vectors() {
    xtensa_setup_vectors();
}

extern "C" void xtensa_vector_debug(uint32_t epc1, uint32_t epc2, uint32_t epc3, uint32_t exccause, uint32_t debugcause, uint32_t interrupt) {
    KERNEL_PANIC("debug vector epc1=0x%p epc2=0x%p epc3=0x%p exccause=%u debugcause=0x%p interrupt=0x%p", epc1, epc2, epc2, exccause, debugcause, interrupt);
}

extern "C" void xtensa_vector_nmi(uint32_t epc1, uint32_t epc2, uint32_t epc3, uint32_t exccause, uint32_t excvaddr, uint32_t interrupt) {
    KERNEL_PANIC("NMI vector epc1=0x%p epc2=0x%p epc3=0x%p exccause=%u excvaddr=0x%p interrupt=0x%p", epc1, epc2, epc2, exccause, excvaddr, interrupt);
}

extern "C" void xtensa_kernel_exception(uint32_t epc1, uint32_t epc2, uint32_t epc3, uint32_t exccause, uint32_t excvaddr, uint32_t interrupt) {
    KERNEL_PANIC("kernel exception epc1=0x%p epc2=0x%p epc3=0x%p exccause=%u excvaddr=0x%p interrupt=0x%p", epc1, epc2, epc2, exccause, excvaddr, interrupt);
}

extern "C" void xtensa_user_exception(uint32_t epc1, uint32_t epc2, uint32_t epc3, uint32_t exccause, uint32_t excvaddr, uint32_t interrupt) {
    KERNEL_PANIC("user exception epc1=0x%p epc2=0x%p epc3=0x%p exccause=%u excvaddr=0x%p interrupt=0x%p", epc1, epc2, epc2, exccause, excvaddr, interrupt);
}

extern "C" void xtensa_double_exception(uint32_t epc1, uint32_t epc2, uint32_t epc3, uint32_t exccause, uint32_t excvaddr, uint32_t depc) {
    KERNEL_PANIC("double exception epc1=0x%p epc2=0x%p epc3=0x%p exccause=%u excvaddr=0x%p depc=0x%p", epc1, epc2, epc2, exccause, excvaddr, depc);
}
