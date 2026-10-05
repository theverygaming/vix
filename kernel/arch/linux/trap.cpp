#include <vix/arch/monitor.h>
#include <vix/panic.h>
#include <vix/debug.h>
#include <vix/kprintf.h>
#include <vix/arch/common/cpu.h>
#include <vix/sched.h>

static void dumpregs(struct arch::full_ctx *ctx) {
    kprintf(
        KP_ALERT,
        "dumpregs:"
        " rax: 0x%p rbx: 0x%p rcx: 0x%p rdx: 0x%p rsi: 0x%p rdi: 0x%p rsp: 0x%p rbp: 0x%p"
        " r8: 0x%p r9: 0x%p r10: 0x%p r11: 0x%p r12: 0x%p r13: 0x%p r14: 0x%p r15: 0x%p"
        " rip: 0x%p cs: 0x%p rflags: 0x%p"
        " fs_base: 0x%p gs_base: 0x%p"
        " monitor_flags: 0x%p"
        "\n",
        ctx->rax, ctx->rbx, ctx->rcx, ctx->rdx, ctx->rsi, ctx->rdi, ctx->rsp, ctx->rbp,
        ctx->r8, ctx->r9, ctx->r10, ctx->r11, ctx->r12, ctx->r13, ctx->r14, ctx->r15,
        ctx->rip, ctx->cs, ctx->rflags,
        ctx->fs_base, ctx->gs_base,
        ctx->monitor_flags
    );
}

extern "C" void trap_handler(struct arch::full_ctx *ctx) {
    switch (ctx->interrupt_code) {
        case MONITOR_TRAPCODE_TIMER: {
            DEBUG_PRINTF("timer tick!\n");
            sched::yield();
            break;
        }
        case MONITOR_TRAPCODE_SEGV: {
            dumpregs(ctx);
            KERNEL_PANIC("segmentation fault accessing 0x%p", ctx->interrupt_meta1);
            break;
        }
        case MONITOR_TRAPCODE_ME: {
            dumpregs(ctx);
            KERNEL_PANIC("math fault");
            break;
        }
        default: {
            dumpregs(ctx);
            KERNEL_PANIC("unknown trapcode 0x%p", ctx->interrupt_code);
        }
    }
}
