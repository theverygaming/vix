#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>
#include <vix/sched.h>
#include <vix/config.h>

static void procret() {
    KERNEL_PANIC("returned from process");
}

extern "C" void xtensa_restore_full_ctx();

void sched::arch_init_thread(struct sched::thread *proc, void (*func)()) {
#ifdef CONFIG_XTENSA_PLATFORM_ESP8266
    uint32_t *stack_bottom = (uint32_t *)mm::kmalloc_aligned(1024, 16);
    uint32_t *stack_top = (uint32_t *)((uint8_t *)stack_bottom + 1024);
    uint32_t *stack_top_real = (uint32_t *)((uint8_t *)stack_bottom + 1024);

    stack_top -= 96 / 4; // stack must always be 16-byte aligned
    struct arch::full_ctx *full_ctx = (struct arch::full_ctx *)stack_top;
    for (size_t i = 0; i < 16; i++) {
        full_ctx->aregs[i] = 0;
    }
    full_ctx->aregs[0] = (uint32_t)procret;
    full_ctx->aregs[1] = (uint32_t)stack_top_real;
    full_ctx->pc = (uint32_t)func;
    full_ctx->ps = 0; // all interrupts enabled, kernel mode

    stack_top -= 32 / 4; // stack must always be 16-byte aligned
    struct arch::ctx *ctx = (struct arch::ctx *)stack_top;
    ctx->a0 = (uint32_t)xtensa_restore_full_ctx;
    proc->ctx = ctx;
#endif
}
