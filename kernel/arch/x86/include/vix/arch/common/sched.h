#pragma once
#include <vix/arch/common/paging.h>
#include <vix/arch/multitasking.h>

namespace sched {
    struct arch_task {
        arch::vmm::pt_t pt;
        void *kernel_stack_bottom;
        void *kernel_stack_top;
        bool is_ring_3;
    };
}
