#include <forward_list>
#include <vix/interrupts.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>

std::forward_list<sched::thread *> sched::sched_readyqueue;

static struct sched::thread *current = nullptr;

static volatile bool sched_disabled = true;

static struct sched::thread *get_next() {
    if (unlikely(sched::sched_readyqueue.size() == 0)) {
        KERNEL_PANIC("no processes to run");
    }

    return sched::sched_readyqueue.swap_first_last();
}

static void enter_thread(struct sched::thread *p) {
    current = p;
    p->state = sched::thread::state::RUNNING;
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
    push_interrupt_disable(); // the thread switch is a rather major critical section so we disable interrupts
    struct sched::thread *last = current;
    current = get_next();
    // switching to the same thread would break shit
    if (last == current) {
        pop_interrupt_disable();
        return;
    }
    last->state = sched::thread::state::RUNNABLE;
    current->state = sched::thread::state::RUNNING;
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

struct sched::thread sched::init_thread(void (*func)(), void *data1, void *data2) {
    struct sched::thread t;
    sched::arch_init_thread(&t, func);
    t.state = sched::thread::state::RUNNABLE;
    t.tid = -1;
    t.data1 = data1;
    t.data2 = data2;
    //FIXME: everything except IA-32!! arch_init_thread is supposed to set pushpop_interrupt_state and pushpop_interrupt_count!!!
    //t.pushpop_interrupt_state = arch::INTERRUPT_STATE_DISABLED;
    //t.pushpop_interrupt_count = 1; // initially it'll be popped once!
    return t;
}

int sched::start_thread(struct sched::thread t) {
    static int tid_counter = 0;
    sched::thread *nt = new sched::thread(t);
    nt->tid = tid_counter++;
    push_interrupt_disable();
    sched_readyqueue.push_front(nt);
    pop_interrupt_disable();
    return nt->tid;
}

int sched::start_worker(void (*worker)(void *), void *ctx) {
    return start_thread(init_thread(
        []() {
            void (*worker)(void *) = (void (*)(void *))sched::mythread()->data1;
            worker(sched::mythread()->data2);
            sched::die();
        },
        (void *)worker,
        ctx
    ));
}

struct sched::thread *sched::mythread() {
    return current;
}

static sched::thread *find_by_tid(int tid) {
    for (auto it = sched::sched_readyqueue.begin(); it != sched::sched_readyqueue.end(); it++) {
        if ((*it)->tid == tid) {
            return *it;
        }
    }
    return nullptr;
}

static void cleanup_thread(sched::thread *t) {
    sched::sched_readyqueue.erase_first_eq(t);
    // FIXME: deallocate stack and stuff (but as we may currently be in the affected thread that's hard!)
    delete t;
}

void sched::die() {
    push_interrupt_disable();
    sched::thread *del = mythread();
    current = nullptr;
    // FIXME: we kinda need to deallocate stack and stuff (but as we are currently in the affected thread that's hard!)
    cleanup_thread(del);
    pop_interrupt_disable();
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}

void sched::kill_thread(int tid) {
    if (tid == mythread()->tid) {
        die();
        KERNEL_PANIC("unreachable");
    }
    push_interrupt_disable();
    sched::thread *d = find_by_tid(tid);
    if (d == nullptr) {
        KERNEL_PANIC("requested to kill TID %d but we coudln't find it", tid);
    }
    cleanup_thread(d);
    pop_interrupt_disable();
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
