#include <arch/cpubasics.h>
#include <arch/drivers/pic_8259.h>
#include <arch/idt.h>
#include <arch/isr.h>
#include <arch/multitasking.h>
#include <config.h>
#include <stdio.h>

static void set_pit_freq(int hz) {
    int divisor = 1193180 / hz; /* Calculate our divisor */
    outb(0x43, 0x36);           /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8);   /* Set high byte of divisor */
}

static void clockHandler(isr::registers *regs) {
    multitasking::interruptTrigger(regs);
    drivers::pic::pic8259::eoi((uint8_t)regs->interrupt);
}

void cpubasics::cpuinit() {
    idt::i686_IDT_Initialize();
    isr::i686_ISR_Initialize();
    drivers::pic::pic8259::init(32, 112);
    set_pit_freq(1000);
    isr::RegisterHandler(drivers::pic::pic8259::irqToint(0), clockHandler);
    drivers::pic::pic8259::unmask_irq(0);
    asm volatile("sti");
}
