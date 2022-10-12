#include <cpp.h>
#include <kernel.h>
#include <log.h>
#include <memory_alloc/memalloc.h>
#include INCLUDE_ARCH_GENERIC(startup.h)

#ifdef CONFIG_RUST_SUPPORT
extern "C" uint32_t rust_test();
#endif

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

    arch::generic::startup::stage3_startup();

    // TODO: when arch::generic has a execute function use that to execute init
#ifdef CONFIG_RUST_SUPPORT
    printf("calling rust\n");
    uint32_t rust = rust_test();
    printf("rust returned: %u\n", rust);
#endif

    arch::generic::startup::after_init();

    while (true) {}
}
