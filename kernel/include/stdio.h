#pragma once
#include <config.h>
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
int snprintf(char *s, size_t n, const char *fmt, ...);

