#pragma once
#include <vix/arch/common/cpu.h>
#include <vix/types.h>

namespace isr {
    void i686_ISR_Initialize();

    void RegisterHandler(int, void (*)(struct arch::full_ctx *));
    void DeregisterHandler(int handler);
}
