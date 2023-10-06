#include <arch/common/sched.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <sched.h>

static struct sched::proc *current;

struct proc_ll {
    struct proc_ll *next;
    struct sched::proc proc;
};

static struct proc_ll *ll_begin = nullptr;
static struct proc_ll *ll_last = nullptr;

static struct sched::proc *get_next() {
    if (ll_begin == nullptr) {
        KERNEL_PANIC("no processes to run");
    }

    if (ll_begin->next == nullptr && ll_last != ll_begin) {
        KERNEL_PANIC("linked list skill issue");
    }
    if (ll_last == nullptr) {
        KERNEL_PANIC("linked list skill issue 2");
    }

    if (ll_begin->next == nullptr) {
        return &ll_begin->proc;
    }

    struct sched::proc *p = &ll_begin->proc;
    ll_last->next = ll_begin;
    ll_last = ll_begin;
    ll_begin = ll_begin->next;

    return p;
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
    current = get_next();
    struct arch::ctx *tmp;
    sched_switch(&tmp, current->ctx);
    KERNEL_PANIC("unreachable");
}

void sched::start_thread(void (*func)()) {
    struct sched::proc p;
    sched::init_proc(&p, func);

    struct proc_ll *pl = (struct proc_ll *)mm::kmalloc(sizeof(struct proc_ll));
    pl->proc = p;
    pl->next = ll_begin;
    ll_begin = pl;
    if (ll_last == nullptr) {
        ll_last = ll_begin;
    }
}
