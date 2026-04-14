#pragma once
#include <forward_list>
#include <vix/abi/abi.h>
#include <vix/abi/linux/linux.h>
#include <vix/abi/vix/vix.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/common/sched.h>
#include <vix/config.h>

namespace sched {
    struct thread {
        int tid;
        bool running;
        struct arch::ctx *ctx;

        struct arch_thread thread_arch;

        enum abi::type abi_type = abi::type::KERNEL_ONLY;

#ifdef CONFIG_ENABLE_ABI_LINUX
        struct abi::linux::thread thread_linux;
#endif

#ifdef CONFIG_ENABLE_ABI_VIX
        struct abi::vix::thread thread_vix;
#endif

        // TLS
        void *data1;
        void *data2;

        // Interrupt state (see interrupts.h)
        unsigned int pushpop_interrupt_state;
        unsigned int pushpop_interrupt_count;
    };

    extern std::forward_list<sched::thread *> sched_readyqueue;
    extern std::forward_list<sched::thread *> sched_waitqueue;

    // must be called once - initializes internal data structures
    void init();

    // enters the scheduler - this function will never return
    void __attribute__((noreturn)) enter();

    // should only be called with interrupts disabled
    void yield();

    // allocates stack and stuff, ouput needs to be passed to start_thread later
    struct thread init_thread(void (*func)(), void *data1 = nullptr, void *data2 = nullptr);

    // low-level method to start a thread, allocates and returns TID
    int start_thread(struct thread);

    // high-level method to start a kernel worker thread, returns a TID, the thread will be killed when the function returns
    int start_worker(void (*worker)(void *), void *ctx = nullptr);

    // Returns pointer to thread structure of current running thread
    struct sched::thread *mythread();

    // Called from inside a thread to kill it
    void __attribute__((noreturn)) die();

    void thread_kill(int tid);

    void thread_sleep(int tid);
    void thread_wakeup(int tid);

    // FIXME: we need a proper critical section thingy
    // disables scheduling
    void disable();
    // re-enables scheduling
    void enable();
    // returns true if scheduler is disabled
    bool is_disabled();

    // arch-specific
    void arch_init_thread(struct sched::thread *proc, void (*func)());
}

// arch-specific
extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new, struct sched::thread *prev, struct sched::thread *next);
