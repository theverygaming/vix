#include <vix/status.h>
#include <vix/abi/linux/arch.h>
#include <vix/arch/multitasking.h>
#include <vix/config.h>
#include <vix/mm/mm.h>
#include <vix/arch/common/paging.h>
#include <string.h>
#include <vix/debug.h>

#ifdef CONFIG_ARCH_HAS_PAGING

status::StatusOr<sched::tid_t> abi::linux::arch::start_process(::arch::vmm::pt_t pt, void *entrypoint, std::vector<std::string> *args) {
    void *stack_bottom;

    ::arch::vmm::pt_t prev_pt = ::arch::vmm::get_active_pt();
    ::arch::vmm::load_pt(pt);

    size_t stack_size = 40 * CONFIG_ARCH_PAGE_SIZE;

    ASSIGN_OR_RETURN(
        stack_bottom,
        mm::allocate_non_contiguous(
            stack_size,
            {.user = true, .read_only = false, .no_execute = true, .cache = mm::caching_type::WRITE_BACK},
            {.start = 0, .end = UINTPTR_MAX},
            {.start = CONFIG_ARCH_PAGE_SIZE, .end = CONFIG_KERNEL_HIGHER_HALF}
        )
    );

    DEBUG_PRINTF("stack bottom: 0x%p\n", stack_bottom);

    memset(stack_bottom, 0, stack_size);

    ::arch::vmm::load_pt(prev_pt);

    void *stack_top = (((uint8_t *)stack_bottom) + stack_size);

    return multitasking::create_task(stack_top, entrypoint, pt, args);
}

#endif // CONFIG_ARCH_HAS_PAGING
