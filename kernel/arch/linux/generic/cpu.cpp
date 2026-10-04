#include <vix/arch/generic/cpu.h>
#include <vix/arch/monitor.h>
#include <vix/arch/linux-syscalls.h>

void __attribute__((no_instrument_function)) __attribute__((noreturn)) arch::generic::cpu::halt() {
    while (true) {
        monitor_set_flags(0);
        linux_exit(1);
    }
}
