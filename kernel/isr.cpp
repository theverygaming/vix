#include "isr.h"
#include "idt.h"
#include "stdio.h"
#include "isrs.h"
#include <cstddef>
#include "stdlib.h"

isr::intHandler *handlers = (isr::intHandler*)0x01000000;

extern "C" void i686_ISR_Handler(isr::Registers* regs) {
    if(handlers[regs->interrupt] != NULL) { handlers[regs->interrupt](regs); }
    else if(regs->interrupt >= 32) {
        printf("No interrupt handler for #%lu!, ignoring\n", regs->interrupt);
    }
    else {
        printf("Unhandled Exception #%lu, halting CPU\n", regs->interrupt);
        printf("DUMP:\neax: %u\nebx: %u\necx: %u\nedx: %u\nesi: %u\nedi: %u\nesp: %u\nebp: %u\neip:%u\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp, regs->ebp, regs->eip);
        __asm("hlt");
    }
}

void isr::i686_ISR_Initialize() {
    for(int i = 0; i < 256; i++) {
        handlers[i] = NULL;
    }
    isrs::i686_ISR_InitializeGates();
    for(int i = 0; i < 256; i++) {
        idt::i686_IDT_EnableGate(i);
    }
}


void isr::RegisterHandler(int handler, void (*_func)(Registers* regs)) {
    handlers[handler] = _func;
}

void isr::DeregisterHandler(int handler) {
    handlers[handler] = NULL;
}