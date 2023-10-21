#pragma once
#include <abi/linux/linux.h>
#include <arch/common/cpu.h>
#include <arch/isr.h>
#include <types.h>

#define __SYS_DEFINE(x) uint32_t x(struct arch::full_ctx *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)

abi::linux::syscall_arg_t(sys_dbg_2)(abi::linux::syscall_arg_t a0,
                                     abi::linux::syscall_arg_t a1,
                                     abi::linux::syscall_arg_t a2,
                                     abi::linux::syscall_arg_t a3,
                                     abi::linux::syscall_arg_t a4,
                                     abi::linux::syscall_arg_t a5);

__SYS_DEFINE(sys_dbg);
__SYS_DEFINE(sys_exit);
__SYS_DEFINE(sys_fork);
__SYS_DEFINE(sys_read);
__SYS_DEFINE(sys_write);
__SYS_DEFINE(sys_open);
__SYS_DEFINE(sys_close);
__SYS_DEFINE(sys_waitpid);
__SYS_DEFINE(sys_execve);
__SYS_DEFINE(sys_time);
__SYS_DEFINE(sys_getpid);
__SYS_DEFINE(sys_brk);
__SYS_DEFINE(sys_ioctl);
__SYS_DEFINE(sys_mmap);
__SYS_DEFINE(sys_sysinfo);
__SYS_DEFINE(sys_clone);
__SYS_DEFINE(sys_uname);
__SYS_DEFINE(sys_modify_ldt);
__SYS_DEFINE(sys_mprotect);
__SYS_DEFINE(sys_writev);
__SYS_DEFINE(sys_rt_sigprocmask);
__SYS_DEFINE(sys_getcwd);
__SYS_DEFINE(sys_mmap2);
__SYS_DEFINE(sys_stat64);
__SYS_DEFINE(sys_lstat64);
__SYS_DEFINE(sys_getuid32);
__SYS_DEFINE(sys_fcntl64);
__SYS_DEFINE(sys_futex);
__SYS_DEFINE(sys_set_thread_area);
__SYS_DEFINE(sys_set_tid_address);
__SYS_DEFINE(sys_set_robust_list);
__SYS_DEFINE(sys_rseq);
