#include <vix/arch/generic/cpu.h>

void __attribute__((no_instrument_function)) __attribute__((noreturn)) arch::generic::cpu::halt() {
    while (true) {
        asm volatile("wfi");
    }
}
