#include <mm/kmalloc.h>
#include <panic.h>
#include <sched.h>
#include <types.h>

static void procret() {
    KERNEL_PANIC("returned from process");
}

void sched::arch_init_thread(struct sched::task *proc, void (*func)()) {
    uint32_t *stack = (uint32_t *)((uint8_t *)mm::kmalloc(256) + 256);
    proc->ctx.sp = (uint32_t)stack;
    proc->ctx.ra =
        ((uint32_t)func & (~(0b11ul << 30))) | 2 << 30; // The two MSB's of the return address are the callx instruction used - in this case call8
}
