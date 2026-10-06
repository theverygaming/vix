#pragma once
#include <vix/status.h>
#include <vix/arch/common/paging.h>
#include <vector>
#include <string>
#include <vix/sched.h>

namespace abi::linux::arch {
    status::StatusOr<sched::tid_t> start_process(::arch::vmm::pt_t pt, void *entrypoint, std::vector<std::string> *args);
}
