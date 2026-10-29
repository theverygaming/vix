#pragma once
#include <string>
#include <vector>
#include <vix/arch/common/cpu.h>
#include <vix/mm/mm.h>
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
    sched::tid_t create_task(void *stackadr, void *entrypoint, mm::mem_areas_t areas, std::vector<std::string> *argv);
#endif
}
