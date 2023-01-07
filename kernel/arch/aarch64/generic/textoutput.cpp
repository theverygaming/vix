#include <arch/drivers/uart.h>
#include <arch/generic/textoutput.h>

void arch::generic::textoutput::puts(const char *str, color foreground, color background) {
    while (*str) {
        drivers::uart::putc(*str++);
    }
}
