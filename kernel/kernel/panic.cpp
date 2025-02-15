#include <vix/arch/generic/cpu.h>
#include <vix/interrupts.h>
#include <vix/kprintf.h>
#include <vix/panic.h>
#include <vix/sched.h>

void kernel_panic(const char *panic_fmt, ...) {
    sched::disable();
    push_interrupt_disable(); // it's over. No need to pop this

    kprintf(KP_EMERG, "KERNEL PANIC!\n");

    va_list args;
    va_start(args, panic_fmt);
    vkprintf(KP_EMERG, panic_fmt, args);
    va_end(args);

    while (true) {
        __builtin_trap();
        arch::generic::cpu::halt();
    }
}
