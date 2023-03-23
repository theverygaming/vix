#include <arch/common/cpu.h>
#include <arch/cpubasics.h>
#include <arch/drivers/pic_8259.h>
#include <arch/idt.h>
#include <arch/isr.h>
#include <arch/multitasking.h>
#include <config.h>
#include <stdio.h>
#include <time.h>

static void set_pit_freq(int hz) {
    int divisor = 1193180 / hz; /* Calculate our divisor */
    outb(0x43, 0x36);           /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8);   /* Set high byte of divisor */
}

static void clockHandler(struct arch::cpu_ctx *regs) {
    time::ms_since_bootup++;
    uint8_t interrupt = (uint8_t)regs->interrupt; // multitasking may change this value, so we save it
    multitasking::interruptTrigger(regs);
    drivers::pic::pic8259::eoi(interrupt);
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
