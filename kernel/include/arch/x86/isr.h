#pragma once
#include "types.h"

namespace isr {
    typedef struct {
        uint32_t intStackLocation;
        uint32_t ds;
        uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
        uint32_t interrupt, error;
        uint32_t eip, cs, eflags, esp, ss; // stuff CPU pushed
    } __attribute__((packed)) Registers;
    typedef void (*intHandler)(Registers *regs);

    void i686_ISR_Initialize();

    void RegisterHandler(int, void (*)(Registers *));
    void DeregisterHandler(int handler);
}
