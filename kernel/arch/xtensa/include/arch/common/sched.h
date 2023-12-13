#pragma once
#include <string.h>
#include <types.h>

namespace sched {
    struct arch_task {};
    struct __attribute__((packed)) xtensa_sched_ctx {
        uint32_t ra;
        uint32_t sp;
    };
}

#define SCHED_ARCH_HAS_CUSTOM_SWITCH
#define SCHED_ARCH_CUSTOM_SWITCH_YIELD_IGNORE_IF(task_prev, task_next) \
    (memcmp(&task_prev->ctx, &task_next->ctx, sizeof(struct sched::xtensa_sched_ctx)))
#define SCHED_ARCH_CUSTOM_SWITCH_STRUCT_TASK_CTX_DEF   struct sched::xtensa_sched_ctx ctx
#define SCHED_ARCH_CUSTOM_SWITCH(task_prev, task_next) sched_switch(&task_prev->ctx, &task_next->ctx)
#define SCHED_ARCH_CUSTOM_SWITCH_ENTRY(task_prev, task_next) \
    do {                                                     \
        sched::xtensa_sched_ctx tmp;                         \
        sched_switch(&tmp, &current->ctx);                   \
    } while (0)

extern "C" void sched_switch(struct sched::xtensa_sched_ctx *old, struct sched::xtensa_sched_ctx *_new);
