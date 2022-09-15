#pragma once
#include <arch/x86/isr.h>

namespace syscall {
    void syscallHandler(isr::Registers *regs);
}