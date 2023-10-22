#pragma once
#include <abi/linux/linux.h>
#include <arch/common/cpu.h>
#include <arch/common/sched.h>
#include <forward_list>

// arch-specific
extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);

namespace sched {

    struct task {
        int pid;
        enum class state { RUNNING, RUNNABLE } state;
        struct arch::ctx *ctx;

        struct arch_task task_arch;

        struct abi::linux::task task_linux;

        // TLS
        void *data;
    };

    extern std::forward_list<sched::task> sched_readyqueue;

    // must be called once - initializes internal data structures
    void init();

    // enters the scheduler - this function will never return
    void __attribute__((noreturn)) enter();

    // should only be called with interrupts disabled
    void yield();

    struct task init_thread(void (*func)(), void *data = nullptr);

    void start_thread(void (*func)(), void *data = nullptr);
    void start_thread(struct task);

    // Returns PID of current running thread
    int mypid();

    // Returns pointer to task structure of current running task
    struct sched::task *myproc();

    // Called from inside a thread to kill it
    void __attribute__((noreturn)) die();

    // FIXME: we need a proper critical section thingy
    // disables scheduling
    void disable();
    // re-enables scheduling
    void enable();

    // arch-specific
    void arch_init_thread(struct sched::task *proc, void (*func)());
}
