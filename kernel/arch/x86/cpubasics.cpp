#include <vix/arch/common/cpu.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/drivers/pic_8259.h>
#include <vix/arch/idt.h>
#include <vix/arch/isr.h>
#include <vix/arch/multitasking.h>
#include <vix/config.h>
#include <vix/kernel/irq.h>
#include <vix/stdio.h>
#include <vix/time.h>

static void set_pit_freq(int hz) {
    int divisor = 1193180 / hz; /* Calculate our divisor */
    outb(0x43, 0x36);           /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8);   /* Set high byte of divisor */
}

static void clockHandler() {
    time::ns_since_bootup = time::ns_since_bootup + 1000000;
    multitasking::interruptTrigger();
}

void cpubasics::cpuinit() {
    idt::i686_IDT_Initialize();
    isr::i686_ISR_Initialize();
    drivers::pic::pic8259::init(32, 112);
    set_pit_freq(1000);
    irq::register_irq_handler(clockHandler, 0);
    asm volatile("sti");
}
