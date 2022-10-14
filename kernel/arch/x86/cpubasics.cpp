#include <arch/x86/cpubasics.h>
#include <arch/x86/drivers/pic_8259.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/multitasking.h>
#include <config.h>
#include <stdio.h>

static void set_pit_freq(int hz) {
    int divisor = 1193180 / hz; /* Calculate our divisor */
    outb(0x43, 0x36);           /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8);   /* Set high byte of divisor */
}

int ticks = 0;
int count = 1;

void cpubasics::sleep(int ms) {
    ticks = 0;
    count = 1;
    while (ticks < ms) {}
    ticks = 0;
}

static void isr_clock_int() {
    if (count == 1) {
        ticks++;
    }
    *((unsigned volatile char *)((KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET) + 2 * 79 + 160 * 0)) = ticks / 20;
}

void clockHandler(isr::registers *regs) {
    isr_clock_int();
    multitasking::interruptTrigger(regs);
    drivers::pic::pic8259::eoi((uint8_t)regs->interrupt);
}

void cpubasics::cpuinit() {
    idt::i686_IDT_Initialize();
    isr::i686_ISR_Initialize();
    drivers::pic::pic8259::init(32, 40);
    set_pit_freq(1000);
    isr::RegisterHandler(drivers::pic::pic8259::irqToint(0), clockHandler);
    drivers::pic::pic8259::unmask_irq(0);
    asm volatile("sti");
}
