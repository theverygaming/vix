#include <arch/common/sched.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <types.h>

static void procret() {
    KERNEL_PANIC("returned from process");
}

void sched::init_proc(struct sched::proc *proc, void (*func)()) {
    uintptr_t *stack = (uintptr_t *)((uint8_t *)mm::kmalloc(1024) + 1024);
    stack -= 1;
    *stack = (uintptr_t)procret;
    stack -= sizeof(struct arch::ctx) / sizeof(uintptr_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    ctx->ebx = ctx->esi = ctx->edi = ctx->ebp = 0;
    ctx->eip = (uintptr_t)func;
    proc->ctx = ctx;
}
