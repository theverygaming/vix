#include <vix/kernel/irq.h>

void irq::register_irq_handler(void (*handler)(), unsigned int irq) {}

void irq::deregister_irq_handler(unsigned int irq) {}
