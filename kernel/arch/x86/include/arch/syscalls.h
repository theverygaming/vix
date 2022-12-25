#pragma once
#include <arch/isr.h>
#include <types.h>

uint32_t sys_exit(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_fork(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_read(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_write(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_waitpid(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_execve(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_time(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_brk(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_ioctl(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_mmap(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_sysinfo(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_uname(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_modify_ldt(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_mprotect(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_writev(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_rt_sigprocmask(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_getcwd(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_mmap2(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_stat64(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_getuid32(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_set_thread_area(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_set_tid_address(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_set_robust_list(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t sys_rseq(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
