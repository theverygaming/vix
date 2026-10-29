#pragma once

namespace sched {
    struct arch_thread {
        void *kernel_stack_bottom;
        void *kernel_stack_top;
        bool is_ring_3;
    };
}
