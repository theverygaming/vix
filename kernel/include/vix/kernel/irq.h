#pragma once

namespace irq {
    void register_irq_handler(void (*handler)(), unsigned int irq);
    void deregister_irq_handler(unsigned int irq);
}
