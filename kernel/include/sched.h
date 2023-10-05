#pragma once
#include <arch/common/cpu.h>

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

namespace sched {
    struct proc {
        int state; // 0 = not active, 1 = active
        struct arch::ctx *ctx;
    };

    // enters the scheduler - this function will never return
    void __attribute__((noreturn)) enter();

    void yield();

    void start_thread(void (*func)());
}
