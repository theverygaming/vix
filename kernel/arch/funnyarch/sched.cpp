#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>
#include <string.h>
#include <vix/kprintf.h>
#include <vix/arch/sched_helpers.h>

extern "C" void funnyarch_trap_return(struct arch::full_ctx *);

void user_thread_launch() {
    kprintf(KP_INFO,
            "hi from user thread(TID %d) kernel stack top: 0x%p user stack top: 0x%p entry: 0x%p stack entry: 0x%p\n",
            sched::mythread()->tid,
            sched::mythread()->thread_arch.kernel_stack_top,
            sched::mythread()->thread_arch.user_stack_top,
            sched::mythread()->data1,
            sched::mythread()->data2);
    struct arch::full_ctx fullctx;
    memset(&fullctx, 0, sizeof(fullctx));
    fullctx.pcst = (1 << 1) | (1 << 3); // usermode & hardware interrupts enabled
    fullctx.rip = (uint32_t)sched::mythread()->data1;
    fullctx.rsp = (uint32_t)sched::mythread()->data2;
    funnyarch_trap_return(&fullctx);
}

#define THREAD_KERNEL_STACK_SIZE (2048)

// FIXME: lmao we don't deallocate anything ever

void sched::arch_init_thread(struct sched::thread *proc, void (*func)()) {
    // TODO: stack guard page! Also just use a block allocator instead of the heap
    uintptr_t *stack_bottom = (uintptr_t *)mm::kmalloc(THREAD_KERNEL_STACK_SIZE);
    uintptr_t *stack_top = (uintptr_t *)((uint8_t *)stack_bottom + THREAD_KERNEL_STACK_SIZE);
    uintptr_t *stack = stack_top;
    stack -= sizeof(struct arch::ctx) / sizeof(uintptr_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    memset(ctx, 0, sizeof(*ctx));
    ctx->lr = (uintptr_t)func;
    proc->ctx = ctx;

    proc->thread_arch.kernel_stack_bottom = stack_bottom;
    proc->thread_arch.kernel_stack_top = stack_top;
    proc->thread_arch.is_usermode = false;
}

extern "C" void funnyarch_sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next) {
    funnyarch_sched_switch(old, _new);
}
