#include <vix/arch/generic/cpu.h>
#include <vix/config.h>
#include <vix/interrupts.h>
#include <vix/kprintf.h>
#include <vix/panic.h>
#include <vix/sched.h>

#ifdef CONFIG_ARCH_X86
#ifdef CONFIG_ENABLE_KERNEL_32
// FIXME: need stack trace abstraction
void x86_stack_trace();
#endif
#endif

void kernel_panic(const char *panic_fmt, ...) {
    sched::disable();
    push_interrupt_disable(); // it's over. No need to pop this

    kprintf(KP_EMERG, "KERNEL PANIC!\n");

    va_list args;
    va_start(args, panic_fmt);
    vkprintf(KP_EMERG, panic_fmt, args);
    va_end(args);

#ifdef CONFIG_ARCH_X86
#ifdef CONFIG_ENABLE_KERNEL_32
    x86_stack_trace();
#endif
#endif

    while (true) {
        arch::generic::cpu::halt();
    }
}
