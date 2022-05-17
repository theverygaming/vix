#include "cpubasics.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include <cstddef>
#include "stdio.h"

void init_pic(void)
{
    /* Initialisation de ICW1 */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    /* Initialisation de ICW2 */
    outb(0x21, 0x20); /* vecteur de depart = 32 */
    outb(0xA1, 0x70); /* vecteur de depart = 96 */

    /* Initialisation de ICW3 */
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    /* Initialisation de ICW4 */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    /* masquage des interruptions */
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}


void (*clockHandlers[256])();

void set_pit_freq(int hz)
{
    int divisor = 1193180 / hz; /* Calculate our divisor */
    outb(0x43, 0x36);           /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8);   /* Set high byte of divisor */
}

int ticks = 0;
int count = 0;

void cpubasics::sleep(int ms)
{
    ticks = 0;
    count = 1;
    while (ticks < ms)
    {
    }
    ticks = 0;
}

void isr_clock_int()
{
    if (count == 1)
    {
        ticks++;
    }
    *((unsigned char *)(0xB8000 + 2 * 79 + 160 * 0)) = ticks / 20;
}

void clockHandler(isr::Registers *gaming) {
    for(int i = 0; i < 256; i++) {
        if(clockHandlers[i] != NULL) {
            clockHandlers[i]();
        }
    }
    outb(0x20, 0x20);
}

void cpubasics::RegisterClockHandler(int number, void (*_func)()) {
    clockHandlers[number] = _func;
}

void cpubasics::DeregisterClockHandler(int number) {
    clockHandlers[number] = NULL;
}

void cpubasics::cpuinit()
{
    gdt::i686_GDT_Initialize();
    idt::i686_IDT_Initialize();
    isr::i686_ISR_Initialize();
    init_pic();
    set_pit_freq(1000);
    isr::RegisterHandler(32, clockHandler);
    RegisterClockHandler(0, isr_clock_int);
    outb(0x21, 0b11111100); // Enable IRQ 0 and 1
    asm("sti");
}
