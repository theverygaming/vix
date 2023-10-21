#include <forward_list>
#include <interrupts.h>
#include <macros.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <sched.h>

std::forward_list<sched::task> sched::sched_readyqueue;

static struct sched::task *current;

static struct sched::task *get_next() {
    if (unlikely(sched::sched_readyqueue.size() == 0)) {
        KERNEL_PANIC("no processes to run");
    }

    return &sched::sched_readyqueue.swap_first_last();
}

static void enter_thread(struct sched::task *p) {
    current = p;
    p->state = sched::task::state::RUNNING;
    struct arch::ctx *tmp;
    sched_switch(&tmp, current->ctx);
}

void sched::init() {}

void sched::yield() {
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        KERNEL_PANIC("yield called with interrupts enabled");
    }
    push_interrupt_disable();
    struct sched::task *last = current;
    current = get_next();
    if (last->ctx == current->ctx) {
        return;
    }
    last->state = sched::task::state::RUNNABLE;
    current->state = sched::task::state::RUNNING;
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
    struct sched::task p;
    sched::arch_init_proc(&p, func);
    p.state = sched::task::state::RUNNABLE;
    p.pid = pid_counter++;

    sched_readyqueue.push_front(p);
    pop_interrupt_disable();
}

int sched::mypid() {
    return current->pid;
}

struct sched::task *sched::myproc() {
    return current;
}

void sched::die() {
    push_interrupt_disable();
    sched_readyqueue.erase_first_if([](const struct sched::task &e) -> bool { return e.pid == mypid(); });
    pop_interrupt_disable();
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}
