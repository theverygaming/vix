#include <forward_list>
#include <vix/interrupts.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/sched.h>
#include <vix/debug.h>
#include <vix/abi/abi.h>

std::forward_list<sched::thread *> sched::sched_readyqueue;
std::forward_list<sched::thread *> sched::sched_waitqueue;
std::forward_list<sched::thread *> sched::sched_reapqueue;

static struct sched::thread *current = nullptr;

static volatile bool sched_disabled = true;

static int reaper_tid = -1;

static struct sched::thread *get_next() {
    if (unlikely(sched::sched_readyqueue.size() == 0)) {
        KERNEL_PANIC("no threads to run");
    }

    return sched::sched_readyqueue.swap_first_last();
}

static void enter_thread(struct sched::thread *p) {
    current = p;
    p->running = true;
    struct arch::ctx *tmp;
    sched_switch(&tmp, current->ctx, nullptr, current);
}

static void reaper(void *) {
    while (true) {
        DEBUG_PRINTF("sched: reaper awoke\n");
        auto it = sched::sched_reapqueue.begin();
        while (it != sched::sched_reapqueue.end()) {
            sched::thread *t = *it;
            DEBUG_PRINTF("sched: reaping TID %d\n", t->tid);
            it++;
            sched::sched_reapqueue.erase_first_eq(t);
            // FIXME: deallocate stack and stuff

            if (t->abi_thread.hooks != nullptr && t->abi_thread.hooks->dealloc_thread != nullptr) {
                t->abi_thread.hooks->dealloc_thread(t);
            }
            delete t;
        }
        // now we sleep
        DEBUG_PRINTF("sched: reaper going to sleep\n");
        sched::thread_sleep(sched::mythread()->tid);
    }
}

void sched::init() {
    reaper_tid = sched::start_kworker(reaper);
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
    last->running = false;
    current->running = true;
    sched_switch(&last->ctx, current->ctx, last, current);
    // NOTE: after switch this code won't always be running.
    // On thread init often arch-specific thread init code will run.
    // It is responsible for correctly setting the CPU's interrupt state.
    pop_interrupt_disable();
}

void sched::enter() {
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}

struct sched::thread sched::init_thread(void (*func)(), struct abi::thread abi_thread, void *data1, void *data2) {
    struct sched::thread t;
    t.abi_thread = abi_thread;
    sched::arch_init_thread(&t, func);
    t.running = false;
    t.tid = -1;
    t.data1 = data1;
    t.data2 = data2;
    t.pushpop_interrupt_count = 0;
    if (t.abi_thread.hooks != nullptr && t.abi_thread.hooks->init_thread != nullptr) {
        t.abi_thread.hooks->init_thread(&t);
    }
    return t;
}

int sched::start_thread(struct sched::thread t) {
    static int tid_counter = 0;
    sched::thread *nt = new sched::thread(t);
    nt->tid = tid_counter++;
    if (nt->abi_thread.hooks != nullptr && nt->abi_thread.hooks->start_thread != nullptr) {
        nt->abi_thread.hooks->start_thread(nt);
    }
    push_interrupt_disable();
    sched_readyqueue.push_front(nt);
    pop_interrupt_disable();
    return nt->tid;
}

int sched::start_kworker(void (*worker)(void *), void *ctx) {
    return start_thread(init_thread(
        []() {
            void (*worker)(void *) = (void (*)(void *))sched::mythread()->data1;
            worker(sched::mythread()->data2);
            sched::die();
        },
        {
            .type = abi::type::KTHREAD,
            .hooks = nullptr,
            .ctx = nullptr,
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
    for (auto it = sched::sched_waitqueue.begin(); it != sched::sched_waitqueue.end(); it++) {
        if ((*it)->tid == tid) {
            return *it;
        }
    }
    return nullptr;
}

static void thread_reap(sched::thread *t) {
    sched::sched_readyqueue.erase_first_eq(t);
    sched::sched_waitqueue.erase_first_eq(t);
    sched::sched_reapqueue.push_front(t);
    sched::thread_wakeup(reaper_tid);
}

void sched::die() {
    push_interrupt_disable();
    sched::thread *del = mythread();
    current = nullptr;
    thread_reap(del);
    // NOTE: there's no pop_interrupt_disable as that wouldn't do anything here.
    // (the current thread is now nullptr, and that means nothing happens on pop_interrupt_disable)
    // The code running after enter_thread shall ensure the correct interrupt context is restored.
    enter_thread(get_next());
    KERNEL_PANIC("unreachable");
}

void sched::thread_kill(int tid) {
    if (tid == mythread()->tid) {
        die();
        KERNEL_PANIC("unreachable");
    }
    push_interrupt_disable();
    sched::thread *d = find_by_tid(tid);
    if (d == nullptr) {
        KERNEL_PANIC("requested to kill TID %d but we coudln't find it", tid);
    }
    thread_reap(d);
    pop_interrupt_disable();
}

void sched::thread_sleep(int tid) {
    push_interrupt_disable();
    sched::thread *t = find_by_tid(tid);
    if (t == nullptr) {
        KERNEL_PANIC("requested to make TID %d sleep but we coudln't find it", tid);
    }
    sched::sched_readyqueue.erase_first_eq(t);
    sched::sched_waitqueue.push_front(t);
    pop_interrupt_disable();
    if (t == current) {
        yield();
    }
}

void sched::thread_wakeup(int tid) {
    push_interrupt_disable();
    sched::thread *t = find_by_tid(tid);
    if (t == nullptr) {
        KERNEL_PANIC("requested to wake up TID %d but we coudln't find it", tid);
    }
    sched::sched_waitqueue.erase_first_eq(t);
    sched::sched_readyqueue.push_front(t);
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
