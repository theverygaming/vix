#pragma once

#define MONITOR_CALL 0xABCD
#define MONITOR_CALL_TRAPRET 0

#define MONITOR_FLAG_USERMODE (1 << 0)
#define MONITOR_FLAG_TIMER (1 << 1)

#ifndef __ASSEMBLER__

#include <vix/arch/linux-syscalls.h>

void monitor_entry(linux_pid_t childpid, int memfd, size_t memfd_bytes);

void monitor_set_trap_handler(uintptr_t addr);
void monitor_set_flags(unsigned long flags);
void monitor_unset_flags(unsigned long flags);

#endif
