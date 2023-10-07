#include <arch/generic/memory.h>
#include <arch/generic/startup.h>
#include <config.h>
#include <debug.h>
#include <kernel.h>
#include <kprintf.h>
#include <libcxx.h>
#include <mm/kmalloc.h>
#include <mm/kvmm.h>
#include <mm/phys.h>
#include <sched.h>
#include <types.h>

#ifdef CONFIG_ARCH_X86
#include <arch/drivers/pci.h>
#endif

#ifdef CONFIG_ENABLE_TESTS
void run_all_tests();
#endif

static void testthread() {
    static int n = 0;
    n++;
    int self = n;
    int i = 0;
    while (true) {
        kprintf(KP_INFO, "i am process %d\n", self);
        sched::yield();
        if (i > 3 && self > 2) {
            kprintf(KP_INFO, "process %d dying...\n", self);
            while (true) {
                sched::yield();
            }
        }
        i++;
    }
}

void kernelstart() {
    kprintf(KP_INFO, "kmain: starting vix -- built " __DATE__ " " __TIME__ "\n");
    mm::phys::phys_init();
#ifdef CONFIG_ARCH_HAS_PAGING
    mm::kv::init();
#endif
    arch::generic::startup::stage2_startup();
    cpp_init();
    kprintf(KP_INFO, "kmain: initialized C++\n");

    arch::generic::startup::stage3_startup();

#ifdef CONFIG_ENABLE_TESTS
    run_all_tests();
#endif

#ifdef CONFIG_ARCH_X86
    drivers::pci::init();
#endif

    size_t freemem = (mm::phys::phys_get_free_blocks() * ARCH_PAGE_SIZE) / 1024;
    char unit = 'K';
    if (freemem >= 10000) {
        unit = 'M';
        freemem /= 1024;
    }
    kprintf(KP_INFO, "kmain: free physical memory: %u%ciB\n", freemem, unit);

    arch::generic::startup::after_init();

    kprintf(KP_INFO, "kmain: entering scheduler\n");
    sched::init();

    for (int i = 0; i < 10; i++) {
        sched::start_thread(testthread);
    }

    sched::enter();
}
