#pragma once
#include <arch/common/cpu.h>
#include <forward_list>

extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

namespace sched {

    struct linux_task {};

    struct task {
        int pid;
        enum class state { RUNNING, RUNNABLE } state;
        struct arch::ctx *ctx;

        struct linux_task task_linux;
    };

    extern std::forward_list<sched::task> sched_readyqueue;

    // must be called once - initializes internal data structures
    void init();

    // enters the scheduler - this function will never return
    void __attribute__((noreturn)) enter();

    // should only be called with interrupts disabled
    void yield();

    void start_thread(void (*func)());

    // Returns PID of current running thread
    int mypid();

    // Returns pointer to task structure of current running task
    struct sched::task *myproc();

    // Called from inside a thread to kill it
    void __attribute__((noreturn)) die();
}
