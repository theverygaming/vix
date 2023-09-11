#pragma once
#include <arch/common/cpu.h>

extern "C" void syscallHandler(struct arch::cpu_ctx *regs);
