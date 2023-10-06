#pragma once
#include <arch/common/cpu.h>

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

namespace sched {
    struct proc {
        struct arch::ctx *ctx;
    };

    // must be called once - initializes internal data structures
    void init();

    // enters the scheduler - this function will never return
    void __attribute__((noreturn)) enter();

    void yield();

    void start_thread(void (*func)());
}
