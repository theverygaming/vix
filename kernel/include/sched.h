#pragma once
#include <abi/abi.h>
#include <abi/linux/linux.h>
#include <abi/vix/vix.h>
#include <arch/common/cpu.h>
#include <arch/common/sched.h>
#include <forward_list>

#ifndef SCHED_ARCH_HAS_CUSTOM_SWITCH
// arch-specific
extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);
#endif

namespace sched {

    struct task {
        int pid;
        enum class state { RUNNING, RUNNABLE } state;
#ifndef SCHED_ARCH_HAS_CUSTOM_SWITCH
        struct arch::ctx *ctx;
#else
        SCHED_ARCH_CUSTOM_SWITCH_STRUCT_TASK_CTX_DEF;
#endif

        struct arch_task task_arch;

        enum abi::type abi_type;

#ifdef CONFIG_ENABLE_ABI_LINUX
        struct abi::linux::task task_linux;
#endif

#ifdef CONFIG_ENABLE_ABI_VIX
        struct abi::vix::task task_vix;
#endif

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
    // returns true if scheduler is disabled
    bool is_disabled();

    // arch-specific
    void arch_init_thread(struct sched::task *proc, void (*func)());
}
