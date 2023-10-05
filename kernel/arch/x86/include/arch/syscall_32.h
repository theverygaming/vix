#pragma once
#include <arch/common/cpu.h>

extern "C" void syscallHandler(struct arch::full_ctx *regs);
