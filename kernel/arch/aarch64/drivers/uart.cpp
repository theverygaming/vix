#include <arch/drivers/uart.h>

void drivers::uart::putc(char c) {
    while ((*((unsigned volatile int *)0x3F201018)) & 0x20) {}
    *((unsigned volatile int *)0x3F201000) = c;
    if (c == '\n') {
        while ((*((unsigned volatile int *)0x3F201018)) & 0x20) {}
        *((unsigned volatile int *)0x3F201000) = '\r';
    }
}
