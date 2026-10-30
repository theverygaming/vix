#include <stdint.h>
#include <vix/macros.h>
#include <vix/status.h>
#include <vix/abi/vix/arch.h>
#include <vix/arch/sched_helpers.h>

status::StatusOr<sched::tid_t> abi::vix::arch::start_thread(mm::mem_areas_t areas, void *entrypoint, std::vector<std::string> *args) {
    void *token = mm::mem_areas_tmp_load(&areas);

    size_t stack_size = 2 * CONFIG_ARCH_PAGE_SIZE;

    void *stack_bottom;
    // TODO: we could ASSIGN_OR_RETURN but we need to call mm::mem_areas_tmp_restore this must be implemented!
    ASSIGN_OR_PANIC(stack_bottom, mm::mem_areas_alloc_range(
        &areas,
        stack_size,
        {.user = true, .read_only = false, .no_execute = true, .cache = mm::caching_type::WRITE_BACK},
        {.start = 0, .end = UINTPTR_MAX} // TODO: NOTE: if we enable MMU on funnyarch this needs to be set differently!
    ));

    DEBUG_PRINTF("stack bottom: 0x%p\n", stack_bottom);

    memset(stack_bottom, 0, stack_size);

    void *stack_top = ((uint8_t *)stack_bottom) + stack_size;

    uint32_t *stack_entry = (uint32_t *)stack_top;

    // argument strings
    uint8_t *arg_strs_start = (uint8_t *)stack_entry;
    uint8_t *arg_strs_current = arg_strs_start;
    for (size_t i = 0; i < args->size(); i++) {
        size_t arglen = (*args)[i].size() + 1;
        arg_strs_current -= arglen;
        memcpy(arg_strs_current, (*args)[i].c_str(), arglen);
        arg_strs_current = PTR_ALIGN_DOWN(arg_strs_current, sizeof(uint32_t));
    }
    stack_entry = (uint32_t *)arg_strs_current;

    // environment pointers
    // null termination
    stack_entry--;
    *stack_entry = 0;

    // arg pointers
    arg_strs_current = arg_strs_start;
    for (size_t i = 0; i < args->size(); i++) {
        size_t arglen = (*args)[i].size() + 1;
        arg_strs_current -= arglen;
        stack_entry--;
        *stack_entry = (uint32_t)(uintptr_t)arg_strs_current;
        arg_strs_current = PTR_ALIGN_DOWN(arg_strs_current, sizeof(uint32_t));
    }
    // null termination
    stack_entry--;
    *stack_entry = 0;

    // argc
    stack_entry--;
    *stack_entry = args->size();

    mm::mem_areas_tmp_restore(token);

    struct sched::thread t = sched::init_thread(&user_thread_launch, {
        .type = abi::type::VIX,
        .hooks = nullptr,
        .ctx = nullptr,
    }, areas, entrypoint, stack_entry);
    t.thread_arch.is_usermode = true;
    t.thread_arch.user_stack_top = stack_top;
    t.thread_arch.user_stack_bottom = stack_bottom;
    return sched::start_thread(t);
}
