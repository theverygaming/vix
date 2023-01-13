#include <arch/generic/startup.h>
#include <config.h>
#include <cpp.h>
#include <kernel.h>
#include <log.h>
#include <mm/kmalloc.h>
#include <mm/phys.h>

#ifdef CONFIG_ENABLE_TESTS
void run_all_tests();
#endif

void kernelstart() {
    LOG_NORMAL("hewwo");
    LOG_NORMAL("starting shitOS -- built " __DATE__ " " __TIME__);
    mm::phys::phys_init();
    LOG_NORMAL("initialized physical memory manager");
    mm::phys::kernel_init();
    LOG_NORMAL("initialized kernel memory manager");
    arch::generic::startup::stage2_startup();
    LOG_NORMAL("initializing C++");
    cpp_init();
    LOG_NORMAL("initialized C++");

    arch::generic::startup::stage3_startup();

#ifdef CONFIG_ENABLE_TESTS
    run_all_tests();
#endif

    arch::generic::startup::after_init();

    while (true) {}
}
