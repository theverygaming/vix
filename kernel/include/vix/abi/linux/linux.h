#pragma once
#include <vix/types.h>
#include <vix/status.h>
#include <vector>
#include <string>
#include <vix/sched.h>

namespace abi::linux {
    struct thread {};
    typedef uintptr_t syscall_arg_t;
    typedef uintptr_t syscall_return_t;

    status::StatusOr<sched::tid_t> exec(const char *path, std::vector<std::string> *args);
}
