#pragma once
#include <vix/arch/common/cpu.h>

void dispatch_syscall(struct arch::full_ctx *regs);
