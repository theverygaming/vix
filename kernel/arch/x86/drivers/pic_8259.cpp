#include <vix/arch/cpubasics.h>
#include <vix/arch/drivers/pic_8259.h>
#include <vix/debug.h>

static uint8_t _master_base;
static uint8_t _slave_base;

namespace drivers::pic::pic8259 {
    void init(uint8_t master_base, uint8_t slave_base) {
        assertm((master_base % 8) == 0, "8259 pic offsets must be divisible by 8!");
        assertm((slave_base % 8) == 0, "8259 pic offsets must be divisible by 8!");

        /* Initialisation de ICW1 */
        outb(0x20, 0x11);
        outb(0xA0, 0x11);

        /* Initialisation de ICW2 */
        outb(0x21, master_base);
        _master_base = master_base;
        outb(0xA1, slave_base);
        _slave_base = slave_base;

        /* Initialisation de ICW3 */
        outb(0x21, 0x04);
        outb(0xA1, 0x02);

        /* Initialisation de ICW4 */
        outb(0x21, 0x01);
        outb(0xA1, 0x01);

        /* masquage des interruptions */
        outb(0x21, 0xFB); // 0xFB because slave PIC
        outb(0xA1, 0xFF);
    }

    void mask_irq(uint8_t irqnum) {
        if (irqnum == 2) { // slave PIC connected here
            return;
        }
        if (irqnum > 7) {
            uint8_t current = inb(0xA1);
            current |= 1 << irqnum;
            outb(0xA1, current);
            return;
        }
        uint8_t current = inb(0x21);
        current |= 1 << irqnum;
        outb(0x21, current);
    }

    void unmask_irq(uint8_t irqnum) {
        if (irqnum == 2) { // slave PIC connected here
            return;
        }
        if (irqnum > 7) {
            uint8_t current = inb(0xA1);
            current &= ~(1 << (irqnum - 8));
            outb(0xA1, current);
            return;
        }
        uint8_t current = inb(0x21);
        current &= ~(1 << irqnum);
        outb(0x21, current);
    }

    void eoi(uint8_t intnum) {
        if (!(((intnum >= _master_base) && (intnum < (_master_base + 8))) || ((intnum >= _slave_base) && (intnum < (_slave_base + 9))))) {
            return;
        }

        if ((intnum > _slave_base) && (intnum < (_slave_base + 9))) {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
    }

    uint8_t irqToint(uint8_t irq) {
        if (irq > 7) {
            return _slave_base + (irq - 8);
        }
        return _master_base + irq;
    }
}
