#pragma once
#include <vix/arch/multitasking.h>
#include <vector>

namespace sched {
    struct arch_task {
        std::vector<multitasking::process_pagerange> pages;
    };
}
