#include <vix/status.h>
#include <vix/abi/linux/arch.h>
#include <vix/arch/multitasking.h>
#include <vix/config.h>
#include <vix/mm/mm.h>
#include <vix/arch/common/paging.h>
#include <string.h>
#include <vix/debug.h>

#ifdef CONFIG_ARCH_HAS_PAGING

status::StatusOr<sched::tid_t> abi::linux::arch::start_process(mm::mem_areas_t areas, void *entrypoint, std::vector<std::string> *args) {
    void *stack_bottom;

    void *token = mm::mem_areas_tmp_load(&areas);

    size_t stack_size = 40 * CONFIG_ARCH_PAGE_SIZE;

    // TODO: we could ASSIGN_OR_RETURN but we need to call mm::mem_areas_tmp_restore this must be implemented!
    ASSIGN_OR_PANIC(stack_bottom, mm::mem_areas_alloc_range(
        &areas,
        stack_size,
        {.user = true, .read_only = false, .no_execute = true, .cache = mm::caching_type::WRITE_BACK},
        {.start = CONFIG_ARCH_PAGE_SIZE * 100, .end = CONFIG_KERNEL_HIGHER_HALF}
    ));

    DEBUG_PRINTF("stack bottom: 0x%p\n", stack_bottom);

    memset(stack_bottom, 0, stack_size);

    mm::mem_areas_tmp_restore(token);

    void *stack_top = (((uint8_t *)stack_bottom) + stack_size);

    return multitasking::create_task(stack_top, entrypoint, areas, args);
}

#endif // CONFIG_ARCH_HAS_PAGING
