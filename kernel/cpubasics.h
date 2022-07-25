#pragma once
#include "types.h"

/* IO ports */
#define outb(port, value) asm volatile("outb %%al, %%dx" ::"d"(port), "a"(value))
#define outw(port, value) asm volatile("outw %%ax, %%dx" ::"d"(port), "a"(value))
#define outl(port, value) asm volatile("outl %%eax, %%dx" ::"d"(port), "a"(value))

#define inb(port)                                                                                                                                                                                      \
    ({                                                                                                                                                                                                 \
        uint8_t _v;                                                                                                                                                                                    \
        asm volatile("inb %%dx, %%al" : "=a"(_v) : "d"(port));                                                                                                                                         \
        _v;                                                                                                                                                                                            \
    })
    
#define inw(port)                                                                                                                                                                                      \
    ({                                                                                                                                                                                                 \
        uint16_t _v;                                                                                                                                                                                   \
        asm volatile("inw %%dx, %%ax" : "=a"(_v) : "d"(port));                                                                                                                                         \
        _v;                                                                                                                                                                                            \
    })

#define inl(port)                                                                                                                                                                                      \
    ({                                                                                                                                                                                                 \
        uint32_t _v;                                                                                                                                                                                   \
        asm volatile("inl %%dx, %%eax" : "=a"(_v) : "d"(port));                                                                                                                                        \
        _v;                                                                                                                                                                                            \
    })

namespace cpubasics {
    void sleep(int ms);

    void RegisterClockHandler(int number, void (*)());
    void DeregisterClockHandler(int number);

    void cpuinit();
}