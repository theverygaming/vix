#include <arch/common/bootup.h>
#include <arch/generic/memory.h>
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

static void kthread0() {
    kprintf(KP_INFO, "kmain: first kernel thread started (PID %d)\n", sched::mypid());
    arch::startup::kthread0();
    kprintf(KP_INFO, "kmain: first kernel thread dying (PID %d)\n", sched::mypid());
    sched::die();
}

void kernelstart() {
    kprintf(KP_INFO, "kmain: starting vix -- built " __DATE__ " " __TIME__ "\n");
    mm::phys::phys_init();
#ifdef CONFIG_ARCH_HAS_PAGING
    mm::kv::init();
#endif
    arch::startup::stage2_startup();
    cpp_init();
    kprintf(KP_INFO, "kmain: initialized C++\n");

    arch::startup::stage3_startup();

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

    kprintf(KP_INFO, "kmain: entering scheduler\n");
    sched::init();

    sched::start_thread(kthread0);

    sched::enter();
}
