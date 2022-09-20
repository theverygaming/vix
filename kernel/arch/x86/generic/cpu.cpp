#include <arch/x86/generic/cpu.h>

void arch::generic::cpu::halt() {
    while (true) {
        asm volatile("cli");
        asm volatile("hlt");
    }
}
