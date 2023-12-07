#pragma once

#define SCHED_ARCH_HAS_CUSTOM_SWITCH
#define SCHED_ARCH_CUSTOM_SWITCH(task_prev, task_next) sched_switch(&task_prev->ctx, task_next->ctx)
#define SCHED_ARCH_CUSTOM_SWITCH_ENTRY(task_prev, task_next) \
    do {                                                     \
        struct arch::ctx *tmp;                               \
        sched_switch(&tmp, current->ctx);                    \
    } while (0)

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

namespace sched {
    struct arch_task {};
}
