#include <arch/cpubasics.h>
#include <arch/gdt.h>
#include <arch/generic/memory.h>
#include <arch/idt.h>
#include <arch/isr.h>
#include <arch/isrs.h>
#include <arch/multitasking.h>
#include <arch/tss.h>
#include <config.h>
#include <debug.h>
#include <log.h>
#include <mm/memalloc.h>
#include <stdio.h>
#include <stdlib.h>

isr::intHandler handlers[256];

// #define DEBUG_ENTRY_EXIT
#define PANIC_ON_PROGRAM_FAULT

extern "C" void i686_ISR_Handler(isr::registers *regs) {
#ifdef DEBUG_ENTRY_EXIT
    DEBUG_PRINTF_INSANE("KENTER(%u) -- eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp_u: 0x%p esp_p: 0x%p ebp: 0x%p eip: 0x%p\n",
                        regs->interrupt,
                        regs->eax,
                        regs->ebx,
                        regs->ecx,
                        regs->edx,
                        regs->esi,
                        regs->edi,
                        regs->esp_user,
                        regs->esp_pusha,
                        regs->ebp,
                        regs->eip);
#endif
    uint32_t previous_esp_user = regs->esp_user;
    // TSS stuff
    tss::tss_entry.ss0 = i686_GDT_DATA_SEGMENT;
    tss::tss_entry.esp0 = KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET;

    // get current ring
    uint16_t old_ring = regs->cs & 0x3;
    uint16_t current_ring;
    asm volatile("mov %%cs, %%ax" : "=a"(current_ring) :);
    current_ring &= 0x3;

    if (old_ring != current_ring) {
        // DEBUG_PRINTF("interrupt ring switch! current ring: %u old ring: %u\n", (uint32_t)current_ring, (uint32_t)old_ring);
        // DEBUG_PRINTF("current esp: 0x%p TSS esp: 0x%p\n", regs->esp_user, tss::tss_entry.esp);
        //  KERNEL_PANIC("skill issue");
    } else {
    }

    // is this a spurious IRQ?
    if ((regs->interrupt == 39)) {
        outb(0x20, 0x0b);
        uint8_t reg = inb(0x20);
        if (!((reg >> 7) & 0x1)) {
            LOG_INSANE("Spurious interrupt!");
            return;
        }
    }

    if (handlers[regs->interrupt] != 0) {
        handlers[regs->interrupt](regs);
    } else if (regs->interrupt >= 32) {
        DEBUG_PRINTF("No interrupt handler for #%u!, ignoring\n", regs->interrupt);
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
               regs->esp_user,
               regs->ebp,
               regs->eip);
        if ((regs->eip >= KERNEL_VIRT_ADDRESS) && (regs->eip < KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)) {
            KERNEL_PANIC("kernel page fault");
        }
        printf("Killing current process\n");
        if (multitasking::isProcessSwitchingEnabled()) {
#ifdef PANIC_ON_PROGRAM_FAULT
            KERNEL_PANIC("PANIC_ON_PROGRAM_FAULT");
#endif
            multitasking::killCurrentProcess(regs);
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
               regs->esp_user,
               regs->ebp,
               regs->eip);
        debug::debug_loop();
    } else {
        printf("Exception #%u\n", regs->interrupt);
        printf("Error code: 0x%p\n", regs->error);
        printf("eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n",
               regs->eax,
               regs->ebx,
               regs->ecx,
               regs->edx,
               regs->esi,
               regs->edi,
               regs->esp_user,
               regs->ebp,
               regs->eip);
        if ((regs->eip >= KERNEL_VIRT_ADDRESS) && (regs->eip < KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)) {
            KERNEL_PANIC("kernel exception");
        }
        printf("Killing current process\n");
        if (multitasking::isProcessSwitchingEnabled()) {
#ifdef PANIC_ON_PROGRAM_FAULT
            KERNEL_PANIC("PANIC_ON_PROGRAM_FAULT");
#endif
            multitasking::killCurrentProcess(regs);
        } else {
            debug::debug_loop();
        }
    }

    if (old_ring == current_ring) {}

    uint16_t new_ring = regs->cs & 0x3;

#ifdef DEBUG_ENTRY_EXIT
    DEBUG_PRINTF_INSANE("KEXIT(%u)\n", regs->interrupt);
#endif
    // since we may use a different stack we have to copy some of the data to the old stack
    memcpy((void *)regs->esp_kernel, &regs->eip, 20);
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

void isr::RegisterHandler(int handler, void (*_func)(registers *regs)) {
    handlers[handler] = _func;
}

void isr::DeregisterHandler(int handler) {
    handlers[handler] = 0;
}

extern "C" void *isr_alloc_stack() {
    KERNEL_PANIC("unimplemented");
    /*void *mem = memalloc::page::kernel_malloc(13);
    if (mem == 0) {
        printf("could not allocate memory for syscall!\n");
        debug::debug_loop();
    }
    return ((uint8_t *)mem) + (13 * ARCH_PAGE_SIZE);*/
    return nullptr;
}

extern "C" void isr_free_stack(void *stackadr) {
    KERNEL_PANIC("unimplemented");
    // memalloc::page::kernel_free(((uint8_t *)stackadr) - (13 * ARCH_PAGE_SIZE));
}
