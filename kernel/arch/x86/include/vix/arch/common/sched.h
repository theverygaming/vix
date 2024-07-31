#pragma once
#include <vector>
#include <vix/arch/multitasking.h>

namespace sched {
    struct arch_task {
        std::vector<multitasking::process_pagerange> pages;
    };
}
