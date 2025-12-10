#pragma once
#include <stdarg.h>
#include <vix/config.h>
#include <vix/types.h>

namespace stdio {
    void set_putc_function(void (*putc_function)(char c), bool debugonly = false);
    void unset_putc_function(bool debugonly = false);
}

void putc(char c, bool serialonly = false);
void puts(const char *str, bool serialonly = false);
void printf(const char *fmt, ...);
void printf_serial(const char *fmt, ...);
//int snprintf(char *s, size_t n, const char *fmt, ...);

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

int snprintf(char *s, size_t n, const char *fmt, ...);
int vsnprintf(char *s, size_t n, const char *fmt, va_list arg);

#ifdef __cplusplus
}
#endif
