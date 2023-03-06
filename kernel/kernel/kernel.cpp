#include <arch/generic/startup.h>
#include <config.h>
#include <cpp.h>
#include <debug.h>
#include <kernel.h>
#include <kprintf.h>
#include <mm/kmalloc.h>
#include <mm/kvmm.h>
#include <mm/phys.h>

#ifdef ARCH_X86
#include <arch/drivers/pci.h>
#endif

#ifdef CONFIG_ENABLE_TESTS
void run_all_tests();
#endif

void kernelstart() {
    // TODO: some of these log messages should be in the called functions instead
    kprintf(KP_INFO, "kmain: starting vix -- built " __DATE__ " " __TIME__ "\n");
    mm::phys::phys_init();
    kprintf(KP_INFO, "kmain: initialized physical memory manager\n");
    mm::kv::init();
    kprintf(KP_INFO, "kmain: initialized kernel virtual memory manager\n");
    arch::generic::startup::stage2_startup();
    kprintf(KP_INFO, "kmain: initializing C++\n");
    cpp_init();
    kprintf(KP_INFO, "kmain: initialized C++\n");

    arch::generic::startup::stage3_startup();

#ifdef CONFIG_ENABLE_TESTS
    run_all_tests();
#endif

#ifdef ARCH_X86
    drivers::pci::init();
#endif

    arch::generic::startup::after_init();

    while (true) {}
}
