#include <arch.h>

#include INCLUDE_ARCH_GENERIC(cpu.h)

void arch::generic::cpu::halt() {
    while (true) {}
}

#include INCLUDE_ARCH_GENERIC(memory.h)

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
    return false;
}

#include INCLUDE_ARCH_GENERIC(startup.h)

void arch::generic::startup::stage3_startup() {}

void arch::generic::startup::after_init() {}

#include INCLUDE_ARCH_GENERIC(textoutput.h)

void arch::generic::textoutput::puts(const char *str, color foreground, color background) {}
