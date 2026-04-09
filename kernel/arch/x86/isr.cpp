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
#include <forward_list>

struct int_handler {
    bool (*fn)(void *);
    void *ctx;
};

static std::forward_list<struct int_handler *> *handlers[256];

extern "C" void i686_ISR_Handler(struct arch::full_ctx *regs) {
    // FIXME: holy shit this PIC EOI stuff is a MESS, this clusterfuck can't be working well (2026-04-09)

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

    if (handlers[regs->interrupt] != nullptr) {
        // FIXME: depending on the kind of interrupt we need to take care of EOI far more.. this is BROKEN!
        drivers::pic::pic8259::eoi(regs->interrupt);
        bool handled = false;
        for (auto it = handlers[regs->interrupt]->begin(); it != handlers[regs->interrupt]->end(); it++) {
            if ((*it)->fn((*it)->ctx)) {
                handled = true;
                break;
            }
        }
        if (!handled) {
            kprintf(KP_INFO, "IRQ %u (int %u): nobody cared (we did find a handler list though)\n", irq_n, regs->interrupt);
        }
    } else if (regs->interrupt >= 32) {
        kprintf(KP_INFO, "IRQ %u (int %u): nobody cared\n", irq_n, regs->interrupt);
        // we do need to send an EOI for an unhandled IRQ
        if (is_irq) {
            DEBUG_PRINTF("sending EOI for unhandled IRQ#%u!\n", irq_n);
            drivers::pic::pic8259::eoi(regs->interrupt);
        }
    }

    if ((regs->cs & 0b11) == 3) {
        kprintf(KP_INFO, "going to ring 3\n");
    }
}

void isr::i686_ISR_Initialize() {
    for (int i = 0; i < 256; i++) {
        handlers[i] = nullptr;
    }
    isrs::i686_ISR_InitializeGates();
    for (int i = 0; i < 256; i++) {
        idt::i686_IDT_EnableGate(i);
    }
}

struct irq_handler_handle {
    unsigned int irq;
    struct int_handler *int_handler; 
};

void *irq::register_irq_handler(bool (*handler_fn)(void *), unsigned int irq, void *ctx) {
    unsigned int int_n = drivers::pic::pic8259::irqToint(irq);
    if (handlers[int_n] == nullptr) {
        handlers[int_n] = new std::forward_list<struct int_handler *>;
    }
    struct irq_handler_handle *handle = new irq_handler_handle {
        .irq = irq,
        .int_handler = new int_handler {
            .fn = handler_fn,
            .ctx = ctx,
        },
    };
    handlers[int_n]->push_front(handle->int_handler);
    drivers::pic::pic8259::unmask_irq(irq);
    DEBUG_PRINTF("registered IRQ handler for IRQ %u: fn 0x%p, handle 0x%p\n", irq, handler_fn, handle);
    return handle;
}

void irq::deregister_irq_handler(void *handle) {
    struct irq_handler_handle *handle_i = (struct irq_handler_handle *)handle;
    drivers::pic::pic8259::mask_irq(handle_i->irq);
    handlers[drivers::pic::pic8259::irqToint(handle_i->irq)]->erase_first_eq(handle_i->int_handler);
    DEBUG_PRINTF("deregistered IRQ handler for IRQ %u: fn 0x%p, handle 0x%p\n", handle_i->irq, handle_i->int_handler->fn, handle);
    delete handle_i->int_handler;
    delete handle_i;
}
