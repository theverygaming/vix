#include <string.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/gdt.h>
#include <vix/arch/generic/memory.h>
#include <vix/arch/idt.h>
#include <vix/arch/isr.h>
#include <vix/arch/isrs.h>
#include <vix/arch/multitasking.h>
#include <vix/arch/tss.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/stdio.h>

typedef void (*intHandler)(struct arch::full_ctx *regs);
static intHandler handlers[256];

extern "C" void i686_ISR_Handler(struct arch::full_ctx *regs) {
    uint32_t previous_esp_user = regs->esp;
    // TSS stuff
    // FIXME: this should _not_ be here
    //tss::tss_entry.ss0 = GDT_KERNEL_DATA;
    //tss::tss_entry.esp0 = KERNEL_VIRT_ADDRESS + KERNEL_ISR_STACK_POINTER_OFFSET;
    //tss::tss_entry.esp0 = ((uint32_t)regs) + sizeof(arch::full_ctx);
    if (regs->cs & 0b11 == 3) {
        kprintf(KP_INFO, "going to ring 3\n");
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
