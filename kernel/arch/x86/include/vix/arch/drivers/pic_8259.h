#pragma once
#include <vix/types.h>

namespace drivers::pic::pic8259 {
    void init(uint8_t master_base, uint8_t slave_base);
    void mask_irq(uint8_t irqnum);
    void unmask_irq(uint8_t irqnum);
    void eoi(uint8_t intnum);
    uint8_t irqToint(uint8_t irq);
}
