#pragma once
#include <stdarg.h>
#include <vix/config.h>
#include <vix/types.h>

namespace stdio {
    void set_puts_function(void (*puts_function)(const char *s, size_t n), bool debugonly = false);
    void unset_puts_function(bool debugonly = false);
}

void putc(char c, bool serialonly = false);
void puts(const char *str, bool serialonly = false);
void puts_sized(const char *str, size_t n, bool serialonly = false);
void printf(const char *fmt, ...);

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

int snprintf(char *s, size_t n, const char *fmt, ...);
int vsnprintf(char *s, size_t n, const char *fmt, va_list arg);

#ifdef __cplusplus
}
#endif
