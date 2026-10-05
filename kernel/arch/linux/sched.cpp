#include <vix/mm/mm.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>
#include <string.h>
#include <vix/arch/monitor.h>

extern "C" void trap_return_stack();

#define THREAD_KERNEL_STACK_SIZE (65536)

void sched::arch_init_thread(struct sched::thread *proc, void (*func)()) {
    void *stack_bottom;
    ASSIGN_OR_PANIC(stack_bottom, mm::allocate_non_contiguous(THREAD_KERNEL_STACK_SIZE));
    void *stack_top = ((uint8_t *)stack_bottom + THREAD_KERNEL_STACK_SIZE);
    uint64_t *stack = (uint64_t *)stack_top;

    stack -= sizeof(struct arch::full_ctx) / sizeof(stack[0]);
    struct arch::full_ctx *fullctx = (struct arch::full_ctx *)stack;
    memset(fullctx, 0, sizeof(*fullctx));
    fullctx->rip = (uint64_t)func;
    fullctx->cs = 0x33;
    fullctx->rsp = (uint64_t)stack_top;
    fullctx->monitor_flags = MONITOR_FLAG_TIMER;

    stack -= sizeof(struct arch::ctx) / sizeof(uint64_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    memset(ctx, 0, sizeof(*ctx));
    ctx->rip = (uint64_t)&trap_return_stack;

    proc->ctx = ctx;

    proc->thread_arch.kernel_stack_bottom = stack_bottom;
    proc->thread_arch.kernel_stack_top = stack_top;
    proc->thread_arch.is_usermode = false;
}

extern "C" void core_sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next);

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next) {
    core_sched_switch(old, _new, prev, next);
}
