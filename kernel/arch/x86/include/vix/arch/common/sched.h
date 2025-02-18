#pragma once
#include <vector>
#include <vix/arch/multitasking.h>

namespace sched {
    struct arch_task {
        std::vector<multitasking::process_pagerange> pages;
        void *kernel_stack_bottom;
        void *kernel_stack_top;
        bool is_ring_3;
    };
}
