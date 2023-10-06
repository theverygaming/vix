#include <arch/common/sched.h>
#include <config.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <types.h>

static void procret() {
    KERNEL_PANIC("returned from process");
}

void sched::init_proc(struct sched::proc *proc, void (*func)()) {
#ifdef CONFIG_ENABLE_KERNEL_32
    uint32_t *stack = (uint32_t *)((uint8_t *)mm::kmalloc(1024) + 1024);
    stack -= 1;
    *stack = (uint32_t)procret;
    stack -= sizeof(struct arch::ctx) / sizeof(uint32_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    ctx->ebx = ctx->esi = ctx->edi = ctx->ebp = 0;
    ctx->eip = (uint32_t)func;
    proc->ctx = ctx;
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
