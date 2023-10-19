#include <mm/kmalloc.h>
#include <panic.h>
#include <sched.h>
#include <types.h>

void sched::arch_init_proc(struct sched::task *proc, void (*func)()) {
    uint64_t *stack = (uint64_t *)((uint8_t *)mm::kmalloc(1024) + 1024);
    stack -= sizeof(struct arch::ctx) / sizeof(uint64_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    ctx->x19 = ctx->x20 = ctx->x21 = ctx->x22 = ctx->x23 = ctx->x24 = ctx->x25 = ctx->x26 = ctx->x27 = ctx->x28 = ctx->fp = 0;
    ctx->lr = (uint64_t)func;
    proc->ctx = ctx;
}
