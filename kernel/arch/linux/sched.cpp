#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/types.h>

void sched::arch_init_thread(struct sched::thread *proc, void (*func)()) {
    KERNEL_PANIC("not implemented");
}


extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next) {
    KERNEL_PANIC("not implemented");
}
