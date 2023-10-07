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
    static int pid_counter = 0;
    struct sched::proc p;
    sched::init_proc(&p, func);
    p.pid = pid_counter++;

    readyqueue.push_front(p);
}

int sched::mypid() {
    return current->pid;
}

void sched::die() {
    static struct sched::proc trash_proc;
    readyqueue.remove_if_first([](const struct sched::proc &e) -> bool { return e.pid == mypid(); });
    current = &trash_proc;
    sched::yield();
}
