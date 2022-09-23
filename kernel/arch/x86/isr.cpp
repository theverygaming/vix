#include <arch/x86/cpubasics.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/isrs.h>
#include <arch/x86/multitasking.h>
#include <config.h>
#include <debug.h>
#include <log.h>
#include <memory_alloc/memalloc.h>
#include <stdio.h>
#include <stdlib.h>

isr::intHandler handlers[256];

extern "C" uint32_t i686_ISR_Handler(isr::Registers *regs) {
    // is this a spurious IRQ?
    if ((regs->interrupt == 39)) {
        outb(0x20, 0x0b);
        uint8_t reg = inb(0x20);
        if (!((reg >> 7) & 0x1)) {
            LOG_INSANE("Spurious interrupt!");
            return regs->intStackLocation;
        }
    }

    if (handlers[regs->interrupt] != 0) {
        handlers[regs->interrupt](regs);
    } else if (regs->interrupt >= 32) {
        DEBUG_PRINTF("No interrupt handler for #%lu!, ignoring\n", regs->interrupt);
    } else if (regs->interrupt == 14) {
        uint32_t fault_address;
        asm volatile("mov %%cr2, %0" : "=r"(fault_address)); // get address page fault occoured at
        int present = !(regs->error & 0x1);                  // page not present
        int rw = regs->error & 0x2;                          // is caused by write
        int us = regs->error & 0x4;                          // user or kernel fault?
        int reserved = regs->error & 0x8;                    // reserved bt fuckup?
        int id = regs->error & 0x10;                         // instruction access or data?

        // Output an error message.
        printf("Page fault! ( ");
        if (present) {
            printf("non-present ");
        }
        if (rw) {
            printf("read-only ");
        }
        if (us) {
            printf("user-mode ");
        }
        if (reserved) {
            printf("reserved ");
        }
        if (id) {
            printf("instruction-fetch ");
        }
        printf(") at 0x%p\n", fault_address);
        printf("Error code: 0x%p\n", regs->error);
        printf("eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n",
               regs->eax,
               regs->ebx,
               regs->ecx,
               regs->edx,
               regs->esi,
               regs->edi,
               regs->esp,
               regs->ebp,
               regs->eip);
        printf("Killing current process\n");
        if (multitasking::isProcessSwitchingEnabled()) {
            multitasking::killCurrentProcess();
        } else {
            debug::debug_loop();
        }
    } else if (regs->interrupt == 8 || regs->interrupt == 18) {
        printf("---RIP---\nException #%lu, cannot recover\n", regs->interrupt);
        printf("eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n",
               regs->eax,
               regs->ebx,
               regs->ecx,
               regs->edx,
               regs->esi,
               regs->edi,
               regs->esp,
               regs->ebp,
               regs->eip);
        debug::debug_loop();
    } else {
        printf("Exception #%lu\n", regs->interrupt);
        printf("eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n",
               regs->eax,
               regs->ebx,
               regs->ecx,
               regs->edx,
               regs->esi,
               regs->edi,
               regs->esp,
               regs->ebp,
               regs->eip);
        printf("Killing current process\n");
        if (multitasking::isProcessSwitchingEnabled()) {
            multitasking::killCurrentProcess();
        } else {
            debug::debug_loop();
        }
    }
    return regs->intStackLocation;
}

void isr::i686_ISR_Initialize() {
    for (int i = 0; i < 256; i++) {
        handlers[i] = 0;
    }
    isrs::i686_ISR_InitializeGates();
    for (int i = 0; i < 256; i++) {
        idt::i686_IDT_EnableGate(i);
    }
}

void isr::RegisterHandler(int handler, void (*_func)(Registers *regs)) {
    handlers[handler] = _func;
}

void isr::DeregisterHandler(int handler) {
    handlers[handler] = 0;
}

extern "C" void *isr_alloc_stack() {
    void *mem = memalloc::page::kernel_malloc(13);
    if (mem == 0) {
        printf("could not allocate memory for syscall!\n");
        debug::debug_loop();
    }
    return ((uint8_t *)mem) + (13 * 4096);
}

extern "C" void isr_free_stack(void *stackadr) {
    memalloc::page::kernel_free(((uint8_t *)stackadr) - (13 * 4096));
}
