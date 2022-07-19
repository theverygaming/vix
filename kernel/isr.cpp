#include "isr.h"
#include "idt.h"
#include "stdio.h"
#include "isrs.h"
#include <cstddef>
#include "stdlib.h"
#include "multitasking.h"
#include "../config.h"
#include "debug.h"
#include "memalloc.h"

isr::intHandler *handlers = (isr::intHandler*)(KERNEL_VIRT_ADDRESS + ISR_HANDLER_OFFSET);

extern "C" uint32_t i686_ISR_Handler(isr::Registers* regs) {
    if(handlers[regs->interrupt] != NULL) { handlers[regs->interrupt](regs); }
    else if(regs->interrupt >= 32) {
        printf("No interrupt handler for #%lu!, ignoring\n", regs->interrupt);
    }
    else if(regs->interrupt == 14) { 
        uint32_t fault_address;
        asm volatile("mov %%cr2, %0" : "=r"(fault_address)); // get address page fault occoured at
        int present = !(regs->error & 0x1); // page not present
        int rw = regs->error & 0x2;         // is caused by write
        int us = regs->error & 0x4;         // user or kernel fault?
        int reserved = regs->error & 0x8;   // reserved bt fuckup?
        int id = regs->error & 0x10;        // instruction access or data?

        // Output an error message.
        printf("Page fault! ( ");
        if (present) { printf("non-present "); }
        if (rw) { printf("read-only "); }
        if (us) { printf("user-mode "); }
        if (reserved){ printf("reserved "); }
        printf(") at 0x%p\n", fault_address);
        printf("Error code: 0x%p\n", regs->error);
        printf("eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp, regs->ebp, regs->eip);
        printf("Killing current process\n");
        debug::stack_trace(10, regs->ebp);
        if(multitasking::isProcessSwitchingEnabled()) {
            multitasking::killCurrentProcess();
        }
        else {
            debug::debug_loop();
        }
    }
    else if(regs->interrupt == 8 || regs->interrupt == 18) {
        printf("---RIP---\nException #%lu, cannot recover\n", regs->interrupt);
        printf("eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp, regs->ebp, regs->eip);
        debug::debug_loop();
    }
    else {
        printf("Exception #%lu\n", regs->interrupt);
        printf("eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp, regs->ebp, regs->eip);
        printf("Killing current process\n");
        debug::stack_trace(10, regs->ebp);
        if(multitasking::isProcessSwitchingEnabled()) {
            multitasking::killCurrentProcess();
        }
        else {
            debug::debug_loop();
        }
    }
    return regs->intStackLocation;
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

extern "C" void* isr_alloc_stack() {
    return memalloc::page::kernel_malloc(100) + (100 * 4096); // return top of range
}

extern "C" void isr_free_stack(void* stackadr) {
    // we get top of range
    memalloc::page::kernel_free(stackadr - (100 * 4096));
}