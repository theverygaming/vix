#pragma once
#include <types.h>

/* IO ports */
#define outb(port, value) asm volatile("outb %%al, %%dx" ::"d"(port), "a"(value))
#define outw(port, value) asm volatile("outw %%ax, %%dx" ::"d"(port), "a"(value))
#define outl(port, value) asm volatile("outl %%eax, %%dx" ::"d"(port), "a"(value))

inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %%dx, %%al" : "=a"(value) : "d"(port));
    return value;
}

inline uint16_t inw(uint16_t port) {
    uint16_t value;
    asm volatile("inw %%dx, %%ax" : "=a"(value) : "d"(port));
    return value;
}

inline uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %%dx, %%eax" : "=a"(value) : "d"(port));
    return value;
}

namespace cpubasics {
    void sleep(int ms);

    void RegisterClockHandler(int number, void (*)());
    void DeregisterClockHandler(int number);

    void cpuinit();
}
