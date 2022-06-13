#pragma once
#include "stdio.h"

#define outb(port, value) \
    asm volatile("outb %%al, %%dx" ::"d"(port), "a"(value));


#define outbp(port, value) \
    asm volatile("outb %%al, %%dx; jmp 1f; 1:" ::"d"(port), "a"(value));


#define inb(port) ({              \
    unsigned char _v;             \
    asm volatile("inb %%dx, %%al" \
                 : "=a"(_v)       \
                 : "d"(port));    \
    _v;                           \
})

#define inw(port) ({              \
    unsigned short _v;             \
    asm volatile("inw %%dx, %%ax" \
                 : "=a"(_v)       \
                 : "d"(port));    \
    _v;                           \
})

namespace hdd {
    namespace ata_pio {
        typedef struct __atadevice_t {
            bool dev_okay;
	        char name[41];
        } atadevice_t;
        enum controller {
            ATA_CONTROLLER_PRIMARY = 0x1F0,
            ATA_CONTROLLER_SECONDARY = 0x170,
        };
        enum drive {
            ATA_DRIVE_MASTER = 0xA0,
            ATA_DRIVE_SLAVE = 0xB0,
        };
        void idk();
    }
}