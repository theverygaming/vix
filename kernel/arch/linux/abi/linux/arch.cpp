#include <stdint.h>
#include <vix/macros.h>
#include <vix/status.h>
#include <vix/abi/linux/arch.h>
#include <vix/arch/sched_helpers.h>

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
            {.start = CONFIG_ARCH_PAGE_SIZE * 100, .end = CONFIG_KERNEL_HIGHER_HALF}
        )
    );

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

    // FIXME: 64-bit stack init. 32-bit stuff was bullshit

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

    ::arch::vmm::load_pt(prev_pt);

    struct sched::thread t = sched::init_thread(&user_thread_launch, {
        .type = abi::type::LINUX,
        .hooks = nullptr,
        .ctx = nullptr,
    }, entrypoint, stack_entry);
    t.thread_arch.pt = pt;
    t.thread_arch.is_usermode = true;
    t.thread_arch.user_stack_top = stack_top;
    t.thread_arch.user_stack_bottom = stack_bottom;
    return sched::start_thread(t);
}
