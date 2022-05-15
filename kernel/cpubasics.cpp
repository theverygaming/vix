#include "cpubasics.h"

namespace cpubasics
{


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

    void set_pit_freq(int hz)
    {
        int divisor = 1193180 / hz; /* Calculate our divisor */
        outb(0x43, 0x36);           /* Set our command byte 0x36 */
        outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
        outb(0x40, divisor >> 8);   /* Set high byte of divisor */
    }

    int ticks = 0;
    int count = 0;

    void sleep(int ms)
    {
        ticks = 0;
        count = 1;
        while (ticks < ms)
        {
        }
        ticks = 0;
    }

    void isr_clock_int(void) //this will not work until we manage the IDT
    {
        if (count == 1)
        {
            ticks++;
        }
    }



    void cpuinit()
    {
        init_pic();
    }
}