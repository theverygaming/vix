#pragma once
#include <vix/arch/common/paging.h>

namespace sched {
    struct arch_thread {
        arch::vmm::pt_t pt;
        void *kernel_stack_bottom;
        void *kernel_stack_top;
        bool is_usermode;
        void *user_stack_bottom;
        void *user_stack_top;
    };
}
