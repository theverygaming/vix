#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>
#include <vix/sched.h>

static void procret() {
    KERNEL_PANIC("returned from process");
}

void sched::arch_init_thread(struct sched::thread *proc, void (*func)()) {
    uint32_t *stack_bottom = (uint32_t *)mm::kmalloc_aligned(1024, 16);
    uint32_t *stack_top = (uint32_t *)((uint8_t *)stack_bottom + 1024);
    stack_top -= 32 / 4; // stack must always be 16-byte aligned
    struct arch::ctx *ctx = (struct arch::ctx *)stack_top;
    ctx->a0 = (uint32_t)func;
    proc->ctx = ctx;
}
