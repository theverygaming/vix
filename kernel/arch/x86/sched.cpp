#include <vix/arch/common/cpu.h>
#include <vix/arch/gdt.h>
#include <vix/config.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>

static void procret() {
    KERNEL_PANIC("returned from process");
}

extern "C" void x86_interrupt_return();

void sched::arch_init_thread(struct sched::task *proc, void (*func)()) {
#ifdef CONFIG_ENABLE_KERNEL_32
    uint32_t *stack = (uint32_t *)((uint8_t *)mm::kmalloc(4096) + 4096);

    stack -= sizeof(struct arch::full_ctx) / sizeof(uint32_t);
    struct arch::full_ctx *fullctx = (struct arch::full_ctx *)stack;
    fullctx->eip = (uint32_t)func;
    uint16_t cs = GDT_KERNEL_CODE;
    uint16_t ds = GDT_KERNEL_DATA;
    fullctx->cs = cs;
    fullctx->ds = ds;
    fullctx->es = ds;
    fullctx->fs = ds;
    fullctx->gs = ds;
    fullctx->eflags = 1 << 9; // enable interrupts

    stack -= sizeof(struct arch::ctx) / sizeof(uint32_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    ctx->eflags = 0; // most important thing: interrupts disabled
    ctx->ebx = ctx->esi = ctx->edi = ctx->ebp = 0;
    ctx->eip = (uint32_t)x86_interrupt_return;
    proc->ctx = ctx;
    proc->pushpop_interrupt_state = 1; // interrupts get enabled
    proc->pushpop_interrupt_count = 0;
#endif
#ifdef CONFIG_ENABLE_KERNEL_64
    uint64_t *stack = (uint64_t *)((uint8_t *)mm::kmalloc(1024) + 1024);
    stack -= 1;
    *stack = (uint64_t)procret;
    stack -= sizeof(struct arch::ctx) / sizeof(uint64_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    ctx->r12 = ctx->r13 = ctx->r14 = ctx->r15 = ctx->rbx = ctx->rbp = 0;
    ctx->rip = (uint64_t)func;
    proc->ctx = ctx;
#endif
}
