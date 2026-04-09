#include <vix/kernel/irq.h>

void *irq::register_irq_handler(bool (*handler_fn)(void *), unsigned int irq, void *ctx) {
    return nullptr;
}

void irq::deregister_irq_handler(void *handle) {}
