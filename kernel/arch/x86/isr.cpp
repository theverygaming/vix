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
#include <kprintf.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>

isr::intHandler handlers[256];

// #define DEBUG_ENTRY_EXIT
// #define PANIC_ON_PROGRAM_FAULT

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
    tss::tss_entry.ss0 = GDT_KERNEL_DATA;
    tss::tss_entry.esp0 = KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET;

    // get current ring
    uint16_t old_ring = regs->cs & 0x3;
    uint16_t current_ring;
    asm volatile("mov %%cs, %%ax"
                 : "=a"(current_ring)
                 :);
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
            DEBUG_PRINTF("Spurious interrupt!\n");
            return;
        }
    }

    if (handlers[regs->interrupt] != 0) {
        handlers[regs->interrupt](regs);
    } else if (regs->interrupt >= 32) {
        DEBUG_PRINTF("No interrupt handler for #%u!, ignoring\n", regs->interrupt);
    } else if (regs->interrupt == 14) {
        uint32_t fault_address;
        asm volatile("mov %%cr2, %0"
                     : "=r"(fault_address)); // get address page fault occurred at
        int present = !(regs->error & 0x1);  // page not present
        int rw = regs->error & 0x2;          // is caused by write
        int us = regs->error & 0x4;          // user or kernel fault?
        int reserved = regs->error & 0x8;    // reserved bt fuckup?
        int id = regs->error & 0x10;         // instruction access or data?

        // Output an error message.
        kprintf(KP_ALERT, "isr: Page fault! (\n");
        if (present) {
            kprintf(KP_ALERT, "isr: non-present\n");
        }
        if (rw) {
            kprintf(KP_ALERT, "isr: read-only\n");
        }
        if (us) {
            kprintf(KP_ALERT, "isr: user-mode\n");
        }
        if (reserved) {
            kprintf(KP_ALERT, "isr: reserved\n");
        }
        if (id) {
            kprintf(KP_ALERT, "isr: instruction-fetch\n");
        }
        kprintf(KP_ALERT, "isr: ) at 0x%p\n", fault_address);
        kprintf(KP_ALERT, "isr: Error code: 0x%p\n", regs->error);
        kprintf(KP_ALERT, "isr: eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp_user, regs->ebp, regs->eip);
        if ((regs->eip >= KERNEL_VIRT_ADDRESS) && (regs->eip < KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)) {
            KERNEL_PANIC("kernel page fault");
        }
        kprintf(KP_ALERT, "isr: Killing current process\n");
        if (multitasking::isProcessSwitchingEnabled()) {
#ifdef PANIC_ON_PROGRAM_FAULT
            KERNEL_PANIC("PANIC_ON_PROGRAM_FAULT");
#endif
            multitasking::killCurrentProcess(regs);
        } else {
            KERNEL_PANIC("");
        }
    } else if (regs->interrupt == 8 || regs->interrupt == 18) {
        kprintf(KP_EMERG, "isr: ---RIP---\nException #%lu, cannot recover\n", regs->interrupt);
        kprintf(KP_EMERG, "isr: eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp_user, regs->ebp, regs->eip);
        KERNEL_PANIC("Exception");
    } else {
        kprintf(KP_ALERT, "isr: Exception #%u\n", regs->interrupt);
        kprintf(KP_ALERT, "isr: Error code: 0x%p\n", regs->error);
        kprintf(KP_ALERT, "isr: eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp_user, regs->ebp, regs->eip);
        if ((regs->eip >= KERNEL_VIRT_ADDRESS) && (regs->eip < KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)) {
            KERNEL_PANIC("kernel exception");
        }
        kprintf(KP_ALERT, "isr: Killing current process\n");
        if (multitasking::isProcessSwitchingEnabled()) {
#ifdef PANIC_ON_PROGRAM_FAULT
            KERNEL_PANIC("PANIC_ON_PROGRAM_FAULT");
#endif
            multitasking::killCurrentProcess(regs);
        } else {
            KERNEL_PANIC("");
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
