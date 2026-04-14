#include <forward_list>
#include <vix/interrupts.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>

std::forward_list<sched::task *> sched::sched_readyqueue;

static struct sched::task *current = nullptr;

static volatile bool sched_disabled = true;

static struct sched::task *get_next() {
    if (unlikely(sched::sched_readyqueue.size() == 0)) {
        KERNEL_PANIC("no processes to run");
    }

    return sched::sched_readyqueue.swap_first_last();
}

static void enter_thread(struct sched::task *p) {
    current = p;
    p->state = sched::task::state::RUNNING;
#ifndef SCHED_ARCH_HAS_CUSTOM_SWITCH
    struct arch::ctx *tmp;
    sched_switch(&tmp, current->ctx, nullptr, current);
#else
    SCHED_ARCH_CUSTOM_SWITCH_ENTRY(tmp, current);
#endif
}

void sched::init() {
    sched_disabled = false;
}

void sched::yield() {
    if (unlikely(sched_disabled)) {
        return;
    }
    push_interrupt_disable(); // the task switch is a rather major critical section so we disable interrupts
    struct sched::task *last = current;
    current = get_next();
    // switching to the same task would break shit
    if (last == current) {
        pop_interrupt_disable();
        return;
    }
    last->state = sched::task::state::RUNNABLE;
    current->state = sched::task::state::RUNNING;
#ifndef SCHED_ARCH_HAS_CUSTOM_SWITCH
    sched_switch(&last->ctx, current->ctx, last, current);
#else
    SCHED_ARCH_CUSTOM_SWITCH(last, current);
#endif
    pop_interrupt_disable();
}

void sched::enter() {
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}

struct sched::task sched::init_thread(void (*func)(), void *data1, void *data2) {
    struct sched::task t;
    sched::arch_init_thread(&t, func);
    t.state = sched::task::state::RUNNABLE;
    t.pid = -1;
    t.data1 = data1;
    t.data2 = data2;
    //FIXME: everything except IA-32!! arch_init_thread is supposed to set pushpop_interrupt_state and pushpop_interrupt_count!!!
    //t.pushpop_interrupt_state = arch::INTERRUPT_STATE_DISABLED;
    //t.pushpop_interrupt_count = 1; // initially it'll be popped once!
    return t;
}

int sched::start_thread(struct sched::task t) {
    static int pid_counter = 0;
    sched::task *nt = new sched::task(t);
    nt->pid = pid_counter++;
    push_interrupt_disable();
    sched_readyqueue.push_front(nt);
    pop_interrupt_disable();
    return nt->pid;
}

int sched::start_worker(void (*worker)(void *), void *ctx) {
    return start_thread(init_thread(
        []() {
            void (*worker)(void *) = (void (*)(void *))sched::mytask()->data1;
            worker(sched::mytask()->data2);
            sched::die();
        },
        (void *)worker,
        ctx
    ));
}

struct sched::task *sched::mytask() {
    return current;
}

void sched::die() {
    push_interrupt_disable();
    sched::task *del = mytask();
    sched_readyqueue.erase_first_eq(del);
    current = nullptr;
    // FIXME: we kinda need to deallocate stack and stuff (but as we are currently in the affected thread that's hard!)
    delete del;
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

bool sched::is_disabled() {
    return sched_disabled;
}
