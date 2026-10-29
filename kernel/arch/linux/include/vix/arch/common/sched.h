#pragma once

namespace sched {
    struct arch_thread {
        void *kernel_stack_bottom;
        void *kernel_stack_top;
        bool is_usermode;
        void *user_stack_bottom;
        void *user_stack_top;
    };
}
