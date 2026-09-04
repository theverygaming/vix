#include <vix/interrupts.h>
#include <vix/sched.h>
#include <vix/panic.h>
#include <vix/arch/vectors.h>
#include <vix/kprintf.h>
#include <vix/arch/common/cpu.h>

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

static void dump_excinfo(arch::full_ctx *ctx) {
    for (size_t i = 0; i < 16; i++) {
        kprintf(KP_ALERT, "a%u: 0x%p\n", i, ctx->aregs[i]);
    }
    kprintf(KP_ALERT, "pc: 0x%p\n", ctx->pc);
    kprintf(KP_ALERT, "sar: 0x%p\n", ctx->sar);
    kprintf(KP_ALERT, "ps: 0x%p\n", ctx->ps);
    kprintf(KP_ALERT, "exccause: 0x%p\n", ctx->exccause);
    kprintf(KP_ALERT, "excvaddr: 0x%p\n", ctx->excvaddr);
    kprintf(KP_ALERT, "debugcause: 0x%p\n", ctx->debugcause);
}

extern "C" void xtensa_common_exception(arch::full_ctx *ctx) {
    kprintf(KP_INFO, "exception exccause=0x%p ps=0x%p\n", ctx->exccause, ctx->ps);
    switch (ctx->exccause) {
        case 0x4: // interrupt
            // check which interrupts fired
            uint32_t interrupt;
            asm volatile("rsr.interrupt %0" : "=a"(interrupt));
            kprintf(KP_INFO, "interrupt reg: 0x%p\n", interrupt);
            // clear all interrupts
            asm volatile("wsr.intclear %0" : : "a"(interrupt));
            uint32_t ps;
            asm volatile("rsr.ps %0" : "=a"(ps));
            ps &= ~(1 << 4); // unset EXCM (exception mode)
            asm volatile("wsr.ps %0" : : "a"(ps));
            push_interrupt_disable();
            sched::yield();
            pop_interrupt_disable();
            break;
        default:
            dump_excinfo(ctx);
            KERNEL_PANIC("unknown exception cause");
    }
}
