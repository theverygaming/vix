#pragma once
#include "isr.h"
namespace syscall {
    void syscallHandler(isr::Registers *regs);
}