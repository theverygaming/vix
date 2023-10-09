#include <arch/common/cpu.h>
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

typedef void (*intHandler)(struct arch::full_ctx *regs);
static intHandler handlers[256];

extern "C" void i686_ISR_Handler(struct arch::full_ctx *regs) {
    uint32_t previous_esp_user = regs->esp;
    // TSS stuff
    tss::tss_entry.ss0 = GDT_KERNEL_DATA;
    tss::tss_entry.esp0 = KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET;

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
    }

    // since we may use a different stack we have to copy some of the data to the old stack
    // memcpy((void *)regs->esp_kernel, &regs->eip, 20);
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

void isr::RegisterHandler(int handler, void (*_func)(struct arch::full_ctx *regs)) {
    handlers[handler] = _func;
}

void isr::DeregisterHandler(int handler) {
    handlers[handler] = 0;
}
