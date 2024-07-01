#include <arch/common/bootup.h>
#include <arch/generic/memory.h>
#include <config.h>
#include <debug.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <kprintf.h>
#include <libcxx.h>
#include <mm/kheap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <sched.h>
#include <types.h>

#ifdef CONFIG_ARCH_X86
#include <arch/drivers/pci.h>
#endif

#ifdef CONFIG_ENABLE_TESTS
void run_all_tests();
#endif

#ifdef CONFIG_RUST_SUPPORT
extern "C" uint32_t rust_test(uint32_t);
#endif

static void kthread0() {
    kprintf(KP_INFO, "kmain: first kernel thread started (PID %d)\n", sched::mypid());
    arch::startup::kthread0();
    //fs::vfs::print_tree();
    kprintf(KP_INFO, "kmain: first kernel thread dying (PID %d)\n", sched::mypid());
    sched::die();
}

void kernelstart() {
    kprintf(KP_INFO, "kmain: starting vix -- built " __DATE__ " " __TIME__ "\n");
#ifdef CONFIG_ARCH_HAS_PAGING
    // the PMM needs the VMM to map it's bitmap etc. - so the VMM must be initialized before the PMM
    mm::vmm::init();
#endif
    mm::pmm::init();
    arch::startup::stage2_startup();
    cpp_init();
    kprintf(KP_INFO, "kmain: initialized C++\n");
    fs::vfs::init();
    kprintf(KP_INFO, "kmain: initialized VFS\n");

    arch::startup::stage3_startup();

#ifdef CONFIG_ENABLE_TESTS
    run_all_tests();
#endif

#ifdef CONFIG_ARCH_X86
    drivers::pci::init();
#endif

#ifdef CONFIG_RUST_SUPPORT
    kprintf(KP_INFO, "Calling Rust..\n");
    if (rust_test(5) == 5) {
        kprintf(KP_INFO, "Rust is working!\n");
    } else {
        kprintf(KP_INFO, "Rust returned the wrong value...\n");
    }
#endif

    size_t freemem = (mm::pmm::get_free_blocks() * ARCH_PAGE_SIZE) / 1024;
    char unit = 'K';
    if (freemem >= 10000) {
        unit = 'M';
        freemem /= 1024;
    }
    kprintf(KP_INFO, "kmain: free physical memory: %u%ciB\n", freemem, unit);

    kprintf(KP_INFO, "kmain: initializing scheduler\n");
    sched::init();

    kprintf(KP_INFO, "kmain: starting first scheduler thread\n");
    sched::start_thread(kthread0);

    kprintf(KP_INFO, "kmain: entering scheduler\n");
    sched::enter();
}
