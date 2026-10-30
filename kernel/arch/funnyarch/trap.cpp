#include <vix/panic.h>
#include <vix/kprintf.h>
#include <vix/debug.h>
#include <vix/arch/trap.h>
#include <vix/types.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/syscall.h>
#include <vix/arch/common/interrupts.h>

extern "C" void funnyarch_trap_entry();

void trap_setup() {
    uint32_t ibptr = (uint32_t)&funnyarch_trap_entry;
    ibptr |= 1; // no jump table
    asm volatile("mtsr ibptr, %0" :: "r"(ibptr));
}

static void dumpregs(struct arch::full_ctx *ctx) {
    kprintf(
        KP_ALERT,
        "regs: "
        "r0: 0x%p r1: 0x%p r2: 0x%p r3: 0x%p r4: 0x%p r5: 0x%p r6: 0x%p r7: 0x%p"
        "r8: 0x%p r9: 0x%p r10: 0x%p r11: 0x%p r12: 0x%p r13: 0x%p r14: 0x%p r15: 0x%p"
        "r16: 0x%p r17: 0x%p r18: 0x%p r19: 0x%p r20: 0x%p r21: 0x%p r22: 0x%p r23: 0x%p"
        "r24: 0x%p r25: 0x%p r26: 0x%p rfp: 0x%p lr: 0x%p rsp: 0x%p rip: 0x%p rf: 0x%p\n",
        ctx->r0, ctx->r1, ctx->r2, ctx->r3, ctx->r4, ctx->r5, ctx->r6, ctx->r7,
        ctx->r8, ctx->r9, ctx->r10, ctx->r11, ctx->r12, ctx->r13, ctx->r14, ctx->r15,
        ctx->r16, ctx->r17, ctx->r18, ctx->r19, ctx->r20, ctx->r21, ctx->r22, ctx->r23,
        ctx->r24, ctx->r25, ctx->r26, ctx->rfp, ctx->lr, ctx->rsp, ctx->rip, ctx->rf
    );
}

extern "C" void funnyarch_trap(struct arch::full_ctx *ctx) {
    uint32_t pcst;
    asm volatile("mfsr %0, pcst" : "=r"(pcst));
    uint32_t intnum = pcst >> 24;

    switch (intnum) {
        // vsync interrupt
        case 0: {
            DEBUG_PRINTF("vsync\n");
            break;
        }
        // syscall
        case 0x80: {
            arch::set_interrupt_state(arch::INTERRUPT_STATE_ENABLED);
            syscall_dispatch(ctx);
            break;
        }
        default: {
            dumpregs(ctx);
            KERNEL_PANIC("unknown interrupt %u", intnum);
            break;
        }
    }
}
