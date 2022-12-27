#include <panic.h>
#include <stdio.h>
#include INCLUDE_ARCH_GENERIC(cpu.h)

void __attribute__((no_instrument_function)) __attribute__((noreturn)) panic::kernel_panic(const char *file, int line, const char *text) {
    printf("kernel panic in %s at line %d\n    -> %s\n", file, line, text);

    while (true) {
        arch::generic::cpu::halt();
    }
}
