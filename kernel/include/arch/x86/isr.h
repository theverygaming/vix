#pragma once
#include "types.h"

namespace isr {
    typedef struct {
        uint32_t ds;                                           // data segment pushed by us
        uint32_t edi, esi, ebp, esp_pusha, ebx, edx, ecx, eax; // pusha
        uint32_t esp_kernel;
        uint32_t interrupt, error;                   // we push interrupt, error is pushed automatically (or our dummy)
        uint32_t eip, cs, eflags, esp_user, ss_user; // pushed automatically by CPU
    } __attribute__((packed)) registers;
    typedef void (*intHandler)(registers *regs);

    void i686_ISR_Initialize();

    void RegisterHandler(int, void (*)(registers *));
    void DeregisterHandler(int handler);
}
