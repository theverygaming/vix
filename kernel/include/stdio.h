#pragma once
#include <types.h>

namespace stdio {
    void set_putc_function(void (*putc_function)(char c), bool debugonly = false);
    void unset_putc_function(bool debugonly = false);
}

void putc(char c, bool serialonly = false);
void puts(const char *str, bool serialonly = false);
void putcolor(int x, int y, uint8_t color); // TODO: create generic thingy for this -- currently defined in arch/x86/drivers/text80x25.cpp
void printf(const char *fmt, ...);
void printf_serial(const char *fmt, ...);

typedef __builtin_va_list va_list;
#define va_start(v, l)    __builtin_va_start(v, l)
#define va_end(v)         __builtin_va_end(v)
#define va_arg(v, l)      __builtin_va_arg(v, l)
#define va_copy(dst, src) __builtin_va_copy(dst, src)
