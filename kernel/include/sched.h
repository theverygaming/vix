#pragma once
#include <arch/common/cpu.h>

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

namespace sched {
    struct proc {
        int pid;
        struct arch::ctx *ctx;
    };

    // must be called once - initializes internal data structures
    void init();

    // enters the scheduler - this function will never return
    void __attribute__((noreturn)) enter();

    void yield();

    void start_thread(void (*func)());

    // Returns PID of current running thread
    int mypid();

    // Called from inside a thread to kill it
    void __attribute__((noreturn)) die();
}
