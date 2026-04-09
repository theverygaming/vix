#pragma once

namespace irq {
    // registers a IRQ handler, ctx is context passed to the handler function,
    // the handler function returns true when it handled the IRQ, otherwise false
    // the return value is a handle that can be used to deregister the handler later
    void *register_irq_handler(bool (*handler_fn)(void *), unsigned int irq, void *ctx);

    // deregister a previously registered IRQ handler
    void deregister_irq_handler(void *handle);
}
