#include <forward_list>
#include <interrupts.h>
#include <macros.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <sched.h>

std::forward_list<sched::task> sched::sched_readyqueue;

static struct sched::task *current = nullptr;

static volatile bool sched_disabled = true;

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

void sched::init() {
    sched_disabled = false;
}

void sched::yield() {
    if (unlikely(sched_disabled)) {
        return;
    }
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        KERNEL_PANIC("yield called with interrupts enabled");
    }
    push_interrupt_disable();
    struct sched::task *last = current;
    current = get_next();
    if (last->ctx == current->ctx) {
        pop_interrupt_disable();
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

struct sched::task sched::init_thread(void (*func)(), void *data) {
    static int pid_counter = 0;
    struct sched::task t;
    sched::arch_init_thread(&t, func);
    t.state = sched::task::state::RUNNABLE;
    t.pid = pid_counter++;
    t.data = data;
    return t;
}

void sched::start_thread(struct sched::task t) {
    push_interrupt_disable();
    sched_readyqueue.push_front(t);
    pop_interrupt_disable();
}

void sched::start_thread(void (*func)(), void *data) {
    start_thread(init_thread(func, data));
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

void sched::disable() {
    sched_disabled = true;
}

void sched::enable() {
    sched_disabled = false;
}
