#pragma once
#include <vix/arch/linux-syscalls.h>

void monitor_entry(linux_pid_t childpid, int memfd, size_t memfd_bytes);
int monitor_call();
