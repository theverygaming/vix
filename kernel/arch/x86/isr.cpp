#include <vix/arch/common/cpu.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/drivers/pic_8259.h>
#include <vix/arch/gdt.h>
#include <vix/arch/generic/memory.h>
#include <vix/arch/idt.h>
#include <vix/arch/isr.h>
#include <vix/arch/isrs.h>
#include <vix/arch/multitasking.h>
#include <vix/arch/tss.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/kernel/irq.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/stdio.h>

static struct int_handler {
    void (*fn)();
} handlers[256];

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
    // FIXME: we have some sort of logic for handling this below but atm it's not in working order -> see pic_8259.cpp
    if ((regs->interrupt == 39)) {
        outb(0x20, 0x0b);
        uint8_t reg = inb(0x20);
        if (!((reg >> 7) & 0x1)) {
            DEBUG_PRINTF("Spurious interrupt!\n");
            return;
        }
    }

    bool is_irq = drivers::pic::pic8259::isIntIrq(regs->interrupt);
    uint8_t irq_n = 0;
    if (is_irq) {
        irq_n = drivers::pic::pic8259::intToIrq(regs->interrupt);
    }

    if (is_irq && (irq_n == 7 || irq_n == 15) && drivers::pic::pic8259::checkIrqSpurious(irq_n)) {
        DEBUG_PRINTF("Spurious IRQ #%u, ignoring", irq_n);
        return;
    }

    if (handlers[regs->interrupt].fn != nullptr) {
        // FIXME: depending on the kind of interrupt we need to take care of EOI far more.. this is BROKEN!
        drivers::pic::pic8259::eoi(regs->interrupt);
        handlers[regs->interrupt].fn();
    } else if (regs->interrupt >= 32) {
        DEBUG_PRINTF("No interrupt handler for #%u / IRQ#%u!, ignoring\n", regs->interrupt, irq_n);
        // we do need to send an EOI for an unhandled IRQ
        if (is_irq) {
            DEBUG_PRINTF("sending EOI for unhandled IRQ#%u!\n", irq_n);
            drivers::pic::pic8259::eoi(regs->interrupt);
        }
    }

    // since we may use a different stack we have to copy some of the data to the old stack
    // memcpy((void *)regs->esp_kernel, &regs->eip, 20);
}

void isr::i686_ISR_Initialize() {
    for (int i = 0; i < 256; i++) {
        handlers[i].fn = nullptr;
    }
    isrs::i686_ISR_InitializeGates();
    for (int i = 0; i < 256; i++) {
        idt::i686_IDT_EnableGate(i);
    }
}

void irq::register_irq_handler(void (*handler)(), unsigned int irq) {
    handlers[drivers::pic::pic8259::irqToint(irq)].fn = handler;
    drivers::pic::pic8259::unmask_irq(irq);
}

void irq::deregister_irq_handler(unsigned int irq) {
    drivers::pic::pic8259::mask_irq(irq);
    handlers[drivers::pic::pic8259::irqToint(irq)].fn = nullptr;
}
