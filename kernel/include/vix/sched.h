#pragma once
#include <forward_list>
#include <vix/abi/abi.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/common/sched.h>
#include <vix/config.h>

namespace sched {
    typedef int tid_t;

    struct thread {
        tid_t tid;
        bool running;
        struct arch::ctx *ctx;

        // architecture specific
        struct arch_thread thread_arch;

        // ABI stuff
        struct abi::thread abi_thread;

        // TLS
        void *data1;
        void *data2;

        // Interrupt state (see interrupts.h)
        unsigned int pushpop_interrupt_state;
        unsigned int pushpop_interrupt_count;
    };

    extern std::forward_list<sched::thread *> sched_readyqueue;
    extern std::forward_list<sched::thread *> sched_waitqueue;
    extern std::forward_list<sched::thread *> sched_reapqueue;

    // must be called once - initializes internal data structures
    void init();

    // enters the scheduler - this function will never return
    void __attribute__((noreturn)) enter();

    // should only be called with interrupts disabled
    void yield();

    // allocates stack and stuff, ouput needs to be passed to start_thread later
    struct thread init_thread(void (*func)(), struct abi::thread abi_thread, void *data1 = nullptr, void *data2 = nullptr);

    // low-level method to start a thread, allocates and returns TID
    tid_t start_thread(struct thread);

    // high-level method to start a kernel worker thread, returns a TID, the thread will be killed when the function returns
    int start_kworker(void (*worker)(void *), void *ctx = nullptr);

    // Returns pointer to thread structure of current running thread
    struct sched::thread *mythread();

    // Called from inside a thread to kill it
    void __attribute__((noreturn)) die();

    void thread_kill(tid_t tid);

    void thread_sleep(tid_t tid);
    void thread_wakeup(tid_t tid);

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
