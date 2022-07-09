#pragma once
#include "types.h"

namespace isr {
    typedef struct {
        uint32_t intStackLocation;
        u32 ds;
        u32 edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
        u32 interrupt, error;
        u32 eip, cs, eflags, esp, ss; // stuff CPU pushed
    } __attribute__((packed)) Registers;
    typedef void (*intHandler)(Registers* regs);

    void i686_ISR_Initialize();

    void RegisterHandler(int , void (*)(Registers*));
    void DeregisterHandler(int handler);
}