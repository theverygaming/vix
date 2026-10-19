#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>
#include <string.h>

#define THREAD_KERNEL_STACK_SIZE (2048)

// FIXME: lmao we don't deallocate anything ever

void sched::arch_init_thread(struct sched::thread *proc, void (*func)()) {
    // TODO: stack guard page! Also just use a block allocator instead of the heap
    uintptr_t *stack = (uintptr_t *)((uint8_t *)mm::kmalloc(THREAD_KERNEL_STACK_SIZE) + THREAD_KERNEL_STACK_SIZE);
    stack -= sizeof(struct arch::ctx) / sizeof(uintptr_t);
    struct arch::ctx *ctx = (struct arch::ctx *)stack;
    memset(ctx, 0, sizeof(*ctx));
    ctx->lr = (uintptr_t)func;
    proc->ctx = ctx;
}

extern "C" void funnyarch_sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next) {
    funnyarch_sched_switch(old, _new);
}
