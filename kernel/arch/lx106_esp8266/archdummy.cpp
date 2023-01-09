#include <arch/generic/cpu.h>

void arch::generic::cpu::halt() {
    while (true) {}
}

#include <arch/generic/memory.h>

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
    return false;
}

#include <arch/generic/startup.h>

void arch::generic::startup::stage3_startup() {}

void arch::generic::startup::after_init() {}

#include <arch/generic/textoutput.h>

void arch::generic::textoutput::puts(const char *str, color foreground, color background) {}
