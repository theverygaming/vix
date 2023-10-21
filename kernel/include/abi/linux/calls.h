#pragma once
#include <abi/linux/linux.h>

#define __DECL_LINUX_SYSCALL(name)                                    \
    abi::linux::syscall_arg_t name(abi::linux::syscall_arg_t sysarg0, \
                                   abi::linux::syscall_arg_t sysarg1, \
                                   abi::linux::syscall_arg_t sysarg2, \
                                   abi::linux::syscall_arg_t sysarg3, \
                                   abi::linux::syscall_arg_t sysarg4, \
                                   abi::linux::syscall_arg_t sysarg5)

#define __DEF_LINUX_SYSCALL(name) __DECL_LINUX_SYSCALL(abi::linux::name)

namespace abi::linux {
    __DECL_LINUX_SYSCALL(sys_exit);
    __DECL_LINUX_SYSCALL(sys_read);
    __DECL_LINUX_SYSCALL(sys_write);
}
