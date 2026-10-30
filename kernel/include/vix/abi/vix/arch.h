#pragma once
#include <vix/status.h>
#include <vix/arch/common/paging.h>
#include <vector>
#include <string>
#include <vix/sched.h>
#include <vix/config.h>

namespace abi::vix::arch {
    status::StatusOr<sched::tid_t> start_thread(mm::mem_areas_t areas, void *entrypoint, std::vector<std::string> *args);
}
