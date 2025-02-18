#include <vix/arch/common/cpu.h>
#include <vix/arch/gdt.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>
#ifdef CONFIG_ENABLE_KERNEL_32
#include <vix/arch/multitasking.h>
#include <vix/arch/tss.h>
#endif

static void procret() {
    KERNEL_PANIC("returned from process");
}

extern "C" void x86_interrupt_return();

#define THREAD_KERNEL_STACK_SIZE (65536)

void sched::arch_init_thread(struct sched::task *proc, void (*func)()) {
#ifdef CONFIG_ENABLE_KERNEL_32
    // TODO: stack guard page! Also just use a block allocator instead of the heap
    void *stack_bottom = mm::kmalloc(THREAD_KERNEL_STACK_SIZE);
    void *stack_top = ((uint8_t *)stack_bottom + THREAD_KERNEL_STACK_SIZE);
    uint32_t *stack = (uint32_t *)stack_top;

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
    // TODO: does setting that to 1 make any sense with count = 0?
    proc->pushpop_interrupt_state = 1; // interrupts get enabled
    proc->pushpop_interrupt_count = 0;
    proc->task_arch.kernel_stack_bottom = stack_bottom;
    proc->task_arch.kernel_stack_top = stack_top;
    proc->task_arch.is_ring_3 = false;
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

extern "C" void x86_sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::task *prev, struct sched::task *next);

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::task *prev, struct sched::task *next) {
#ifdef CONFIG_ENABLE_KERNEL_32
    if (prev != nullptr && prev->task_arch.is_ring_3) {
        // check if the TSS ESP0 is correct (this check can be removed later, just for debugging purposes -- maybe make it an assertion?)
        if (tss::tss_entry.esp0 != 0 && (tss::tss_entry.esp0 != (uintptr_t)prev->task_arch.kernel_stack_top)) {
            KERNEL_PANIC("invalid TSS ESP0 - got 0x%p expected 0x%p", tss::tss_entry.esp0, prev->task_arch.kernel_stack_top);
        }
        // maybe before unsetting we should save it? Maybe multitasking::createPageRange(&old_pageranges);?
        multitasking::unsetPageRange(&prev->task_arch.pages);
    }
#endif
#ifdef CONFIG_ENABLE_KERNEL_32
    if (next->task_arch.is_ring_3) {
        tss::tss_entry.esp0 = (uintptr_t)next->task_arch.kernel_stack_top;
        multitasking::setPageRange(&next->task_arch.pages);
    }
#endif
    // silly note: after the switch the parameters passed to this function will be well...
    // not really what you would expect at first since we are now in a different thread!
    x86_sched_switch(old, _new, prev, next);
}
