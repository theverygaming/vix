#include <arch/arch.h>

#include INCLUDE_ARCH_GENERIC(cpu.h)

void arch::generic::cpu::halt() {
    while (true) {}
}

#include INCLUDE_ARCH_GENERIC(memory.h)

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
    return false;
}

#include INCLUDE_ARCH_GENERIC(startup.h)

void arch::generic::startup::stage2_startup() {}

void arch::generic::startup::stage3_startup() {}

void arch::generic::startup::after_init() {}

#include INCLUDE_ARCH_GENERIC(textoutput.h)

void arch::generic::textoutput::puts(const char *str, color foreground, color background) {}

#include INCLUDE_ARCH_GENERIC(devices.h)

arch::generic::devices::get_current_rtc_time(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second) {
    *year = 1970;
    *month = 1;
    *day = 1;
    *hour = 0;
    *minute = 0;
    *second = 0;
}
