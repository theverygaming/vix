#include <arch/common/sched.h>
#include <panic.h>
#include <sched.h>

static struct sched::proc procs[8];
static struct sched::proc *current;

static struct sched::proc *get_next() {
    static int next = 0;
    next &= 0b111; // %= 8;
retry:
    for (int i = next; i < 8; i++) {
        if (procs[i].state == 0) {
            continue;
        }
        next = i + 1;
        return &procs[i];
    }
    if (next == 0) {
        KERNEL_PANIC("no processes to run\n");
    }
    next = 0;
    goto retry;
}

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
    static int n = 0;
    if (n >= 8) {
        KERNEL_PANIC("cannot start any more threads\n");
        return;
    }
    struct sched::proc *p = &procs[n++];
    sched::init_proc(p, func);
    p->state = 1;
}
