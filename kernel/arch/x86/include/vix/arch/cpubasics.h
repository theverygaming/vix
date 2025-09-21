#pragma once
#include <vix/types.h>

/* IO ports */
#define outb(port, value) asm volatile("outb %%al, %%dx" ::"d"(port), "a"(value))
#define outw(port, value) asm volatile("outw %%ax, %%dx" ::"d"(port), "a"(value))
#define outl(port, value) asm volatile("outl %%eax, %%dx" ::"d"(port), "a"(value))

inline __attribute__((always_inline)) uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %%dx, %%al" : "=a"(value) : "d"(port));
    return value;
}

inline __attribute__((always_inline)) uint16_t inw(uint16_t port) {
    uint16_t value;
    asm volatile("inw %%dx, %%ax" : "=a"(value) : "d"(port));
    return value;
}

inline __attribute__((always_inline)) uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %%dx, %%eax" : "=a"(value) : "d"(port));
    return value;
}

namespace cpubasics {
    void cpuinit_early();
    void cpuinit();
}
