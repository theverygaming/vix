#include <arch/common/sched.h>
#include <forward_list>
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
    // kprintf(KP_INFO, "there are %d threads running\n", readyqueue.size());

    return &readyqueue.swap_first_last();
}

static void enter_thread(struct sched::proc *p) {
    current = p;
    struct arch::ctx *tmp;
    sched_switch(&tmp, current->ctx);
}

void sched::init() {}

void sched::yield() {
    struct sched::proc *last = current;
    current = get_next();
    if (last->ctx == current->ctx) {
        return;
    }
    sched_switch(&last->ctx, current->ctx);
}

void sched::enter() {
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}

void sched::start_thread(void (*func)()) {
    struct sched::proc p;
    sched::init_proc(&p, func);

    readyqueue.push_front(p);
}
