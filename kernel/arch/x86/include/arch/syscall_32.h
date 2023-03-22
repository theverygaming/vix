#pragma once
#include <arch/common/cpu.h>

namespace syscall {
    void syscallHandler(struct arch::cpu_ctx *regs);
}
