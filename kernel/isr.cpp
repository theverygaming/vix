#include "isr.h"
#include "idt.h"
#include "stdio.h"
#include "isrs.h"
#include <cstddef>

isr::intHandler handlers[256];

extern "C" void i686_ISR_Handler(isr::Registers* regs) {
    if(handlers[regs->interrupt] != NULL) { handlers[regs->interrupt](regs); }
    else if(regs->interrupt >= 32) {
        printf("No interrupt handler for #%d!, ignoring\n");
    }
    else {
        printf("Unhandled Exception #%d, halting CPU\n");
        __asm("hlt");
    }
}

void isr::i686_ISR_Initialize() {
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