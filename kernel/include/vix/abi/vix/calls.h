#pragma once
#include <vix/abi/vix/vix.h>

#define __DECL_VIX_SYSCALL(name)                                    \
    abi::vix::syscall_arg_t name(abi::vix::syscall_arg_t sysarg0, \
                                   abi::vix::syscall_arg_t sysarg1, \
                                   abi::vix::syscall_arg_t sysarg2, \
                                   abi::vix::syscall_arg_t sysarg3, \
                                   abi::vix::syscall_arg_t sysarg4, \
                                   abi::vix::syscall_arg_t sysarg5)

#define __DEF_VIX_SYSCALL(name) __DECL_VIX_SYSCALL(abi::vix::name)

namespace abi::vix {
    __DECL_VIX_SYSCALL(sys_exit);
}
