#pragma once
#include <arch/isr.h>

namespace syscall {
    void syscallHandler(isr::registers *regs);
}
