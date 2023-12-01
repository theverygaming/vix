#include <mm/kmalloc.h>
#include <panic.h>
#include <sched.h>
#include <types.h>

static void procret() {
    KERNEL_PANIC("returned from process");
}

void sched::arch_init_thread(struct sched::task *proc, void (*func)()) {
    /*uint32_t *stack = (uint32_t *)((uint8_t *)mm::kmalloc(1024) + 1024);
    stack -= 1;
    *stack = (uint32_t)procret;
    stack -= sizeof(struct arch::ctx) / sizeof(uint32_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    ctx->a2 = ctx->a3 = ctx->a4 = ctx->a5 = ctx->a6 = ctx->d2 = ctx->d3 = ctx->d4 = ctx->d5 = ctx->d6 = ctx->d7 = 0;
    ctx->pc = (uint32_t)func;
    proc->ctx = ctx;*/
}
