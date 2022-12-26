#include <arch/drivers/net/rtl8139.h>
#include <arch/drivers/pci.h>
#include <cpp.h>
#include <kernel.h>
#include <log.h>
#include <memory_alloc/memalloc.h>
#include INCLUDE_ARCH_GENERIC(startup.h)

void kernelstart() {
    LOG_NORMAL("hewwo");
    LOG_NORMAL("starting shitOS -- built " __DATE__ " " __TIME__);
    memalloc::page::phys_init();
    LOG_NORMAL("initialized physical memory manager");
    memalloc::page::kernel_init();
    LOG_NORMAL("initialized kernel memory manager");
    arch::generic::startup::stage2_startup();
    cpp_init();
    LOG_NORMAL("initialized C++");

    drivers::pci::init();

    arch::generic::startup::stage3_startup();

    drivers::net::rtl8139::init();

    // arch::generic::startup::after_init();

    while (true) {}
}
