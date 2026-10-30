#include <vix/arch/syscall.h>
#include <vix/abi/vix/vix.h>
#include <vix/arch/common/interrupts.h>
#include <vix/abi/vix/errno.h>
#include <vix/abi/vix/calls.h>

abi::vix::syscall_arg_t (*syscall_table[3])(abi::vix::syscall_arg_t sysarg0,
                                                abi::vix::syscall_arg_t sysarg1,
                                                abi::vix::syscall_arg_t sysarg2,
                                                abi::vix::syscall_arg_t sysarg3,
                                                abi::vix::syscall_arg_t sysarg4,
                                                abi::vix::syscall_arg_t sysarg5) = {
    &abi::vix::sys_exit, // 0
    &abi::vix::sys_read, // 1
    &abi::vix::sys_write, // 2
};

void syscall_dispatch(struct arch::full_ctx *ctx) {
    if (arch::get_interrupt_state() == arch::INTERRUPT_STATE_DISABLED) {
        KERNEL_PANIC("interrupts off in syscall handler");
    }
    if (ctx->r0 <= (sizeof(syscall_table) / sizeof(syscall_table[0]))) {
        DEBUG_PRINTF("calling syscall %u\n", ctx->r0);
        abi::vix::syscall_arg_t retval;
        if (syscall_table[ctx->r0] == nullptr) {
            DEBUG_PRINTF("syscall %u not found\n", ctx->r0);
            ctx->r0 = VIX_ENOSYS;
            return;
        }
        retval = syscall_table[ctx->r0]((abi::vix::syscall_arg_t)ctx->r1,
                                          (abi::vix::syscall_arg_t)ctx->r2,
                                          (abi::vix::syscall_arg_t)ctx->r3,
                                          (abi::vix::syscall_arg_t)ctx->r4,
                                          (abi::vix::syscall_arg_t)ctx->r5,
                                          (abi::vix::syscall_arg_t)ctx->r6);
        DEBUG_PRINTF("syscall return %d\n", retval);
        ctx->r0 = retval;
        return;
    }
    DEBUG_PRINTF("syscall %u not found\n", ctx->r0);
    ctx->r0 = VIX_ENOSYS;
}
