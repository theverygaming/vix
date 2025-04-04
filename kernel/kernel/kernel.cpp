#include <libcxx.h>
#include <vix/arch/common/bootup.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/fs/vfs.h>
#include <vix/initcall.h>
#include <vix/kernel.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/mm/pmm.h>
#include <vix/mm/vmm.h>
#include <vix/sched.h>
#include <vix/types.h>

#ifdef CONFIG_ENABLE_TESTS
void run_all_tests();
#endif

#ifdef CONFIG_RUST_SUPPORT
extern "C" uint32_t rust_test(uint32_t);
#endif

static void kthread0() {
    kprintf(KP_INFO, "kmain: first kernel thread started (PID %d)\n", sched::mytask()->pid);
    arch::startup::kthread0();
    //fs::vfs::print_tree();
    initcall_init_level(INITCALL_DRIVER_INIT);
    kprintf(KP_INFO, "kmain: first kernel thread dying (PID %d)\n", sched::mytask()->pid);
    sched::die();
}

void kernelstart() {
    kprintf(KP_INFO, "kmain: starting vix -- built " __DATE__ " " __TIME__ "\n");
    initcall_init_level(INITCALL_PRE_MM_INIT);
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

    initcall_init_level(INITCALL_EARLY_DRIVER_INIT);

#ifdef CONFIG_ENABLE_TESTS
    run_all_tests();
#endif

#ifdef CONFIG_RUST_SUPPORT
    kprintf(KP_INFO, "Calling Rust..\n");
    if (rust_test(5) == 5) {
        kprintf(KP_INFO, "Rust is working!\n");
    } else {
        kprintf(KP_INFO, "Rust returned the wrong value...\n");
    }
#endif

    size_t freemem = (mm::pmm::get_free_blocks() * CONFIG_ARCH_PAGE_SIZE) / 1024;
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
