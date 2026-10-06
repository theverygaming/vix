#pragma once
#include <string>
#include <vector>
#include <vix/arch/common/cpu.h>
#include <vix/arch/common/paging.h>
#include <vix/arch/isr.h>
#include <vix/arch/paging.h>
#include <vix/config.h>
#include <vix/event.h>
#include <vix/types.h>
#include <vix/sched.h>

namespace multitasking {
    void initMultitasking();
    void interruptTrigger();
#ifdef CONFIG_ARCH_HAS_PAGING
    sched::tid_t create_task(void *stackadr, void *entrypoint, arch::vmm::pt_t pt, std::vector<std::string> *argv);
#endif
}
