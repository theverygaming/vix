#pragma once
#include <vix/config.h>

namespace sched {
    struct thread;
}

namespace abi {
    enum class type {
        KTHREAD,
#ifdef CONFIG_ENABLE_ABI_LINUX
        LINUX,
#endif
#ifdef CONFIG_ENABLE_ABI_VIX
        VIX,
#endif
    };

    struct threadhooks {
        // any of these may be null
        void (*init_thread)(struct sched::thread *);
        void (*start_thread)(struct sched::thread *);
        void (*dealloc_thread)(struct sched::thread *);
    };

    struct thread {
        abi::type type;

        // may be null
        struct threadhooks *hooks;

        // context for the ABI
        void *ctx;
    };
}
