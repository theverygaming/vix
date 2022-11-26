#include <types.h>

static int (*ets_uart_printf)(const char *fmt, ...) = (int (*)(const char *, ...))0x40002544;

extern "C" uint32_t __umodsi3(uint32_t a, uint32_t b) {
    ets_uart_printf("__umodsi3");
    return 5;
}

extern "C" uint32_t __udivsi3(uint32_t a, uint32_t b) {
    ets_uart_printf("__udivsi3");
    return 5;
}

extern "C" uint8_t __atomic_exchange_1(volatile void *mem, uint8_t val, int) {
    uint8_t x = *((uint8_t *)mem);
    *((uint8_t *)mem) = val;
    return x;
}
