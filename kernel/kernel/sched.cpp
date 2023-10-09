#include <arch/common/sched.h>
#include <forward_list>
#include <interrupts.h>
#include <macros.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <sched.h>

static std::forward_list<sched::proc> readyqueue;

static struct sched::proc *current;

static struct sched::proc *get_next() {
    if (unlikely(readyqueue.size() == 0)) {
        KERNEL_PANIC("no processes to run");
    }

    return &readyqueue.swap_first_last();
}

static void enter_thread(struct sched::proc *p) {
    current = p;
    struct arch::ctx *tmp;
    sched_switch(&tmp, current->ctx);
}

void sched::init() {}

void sched::yield() {
    push_interrupt_disable();
    struct sched::proc *last = current;
    current = get_next();
    if (last->ctx == current->ctx) {
        return;
    }
    pop_interrupt_disable();
    sched_switch(&last->ctx, current->ctx);
}

void sched::enter() {
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}

void sched::start_thread(void (*func)()) {
    push_interrupt_disable();
    static int pid_counter = 0;
    struct sched::proc p;
    sched::arch_init_proc(&p, func);
    p.pid = pid_counter++;

    readyqueue.push_front(p);
    pop_interrupt_disable();
}

int sched::mypid() {
    return current->pid;
}

void sched::die() {
    push_interrupt_disable();
    readyqueue.erase_first_if([](const struct sched::proc &e) -> bool { return e.pid == mypid(); });
    pop_interrupt_disable();
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}
