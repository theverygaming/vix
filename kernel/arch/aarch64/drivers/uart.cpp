#include <arch/drivers/gpu/mbox.h>
#include <arch/drivers/uart.h>
#include <types.h>

void drivers::uart::init() {}

void drivers::uart::putc(char c) {
    while ((*((unsigned volatile int *)0x3F201018)) & 0x20) {}
    *((unsigned volatile int *)0x3F201000) = c;
    if (c == '\n') {
        while ((*((unsigned volatile int *)0x3F201018)) & 0x20) {}
        *((unsigned volatile int *)0x3F201000) = '\r';
    }
}
