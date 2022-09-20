#include <arch/arch.h>
#include <config.h>
#include <debug.h>
#include <stdio.h>
#include INCLUDE_ARCH_GENERIC(cpu.h)

void debug::debug_loop() {
    printf("debug deth\n");
    arch::generic::cpu::halt();
}

void debug::breakpoint(int line, char *file) {
    DEBUG_PRINTF("Breakpoint hit in %s at line %d\n", file, line);
}
