#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>

static void (*putc_function_ptr)(char c) = nullptr;
static void (*putc_dbg_function_ptr)(char c) = nullptr;

void stdio::set_putc_function(void (*putc_function)(char c), bool debugonly) {
    if (debugonly) {
        putc_dbg_function_ptr = putc_function;
        return;
    }
    putc_function_ptr = putc_function;
}

void stdio::unset_putc_function(bool debugonly) {
    if (debugonly) {
        putc_dbg_function_ptr = nullptr;
        return;
    }
    putc_function_ptr = nullptr;
}

void putc(char c, bool debugonly) {
    // this is slow as fuck
    if (putc_dbg_function_ptr != nullptr) {
        putc_dbg_function_ptr(c);
    }

    if (debugonly) {
        return;
    }

    if (putc_function_ptr != nullptr) {
        putc_function_ptr(c);
    }
}

void puts(const char *str, bool debugonly) {
    while (*str) {
        putc(*str, debugonly);
        str++;
    }
}

static int printf_base(va_list *args, const char *fmt, char *buf, bool buf_write, size_t buf_len, bool serialonly) {
    size_t chars_written = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case '%': {
                fmt += 1;
                if (buf_write) {
                    size_t can_write = (buf_len - 1) - chars_written;
                    if (chars_written > (buf_len - 1)) {
                        can_write = 0;
                    }
                    if (can_write >= 1) {
                        memcpy(&buf[chars_written], "%", 1);
                    } else {
                        memcpy(&buf[chars_written], "%", can_write);
                    }
                } else {
                    putc('%', serialonly);
                }
                chars_written += 1;
                break;
            }
            case 's': {
                fmt += 1;
                char *arg = va_arg(*args, char *);
                if (buf_write) {
                    size_t can_write = (buf_len - 1) - chars_written;
                    if (chars_written > (buf_len - 1)) {
                        can_write = 0;
                    }
                    if (can_write >= strlen(arg)) {
                        memcpy(&buf[chars_written], arg, strlen(arg));
                    } else {
                        memcpy(&buf[chars_written], arg, can_write);
                    }
                } else {
                    puts(arg, serialonly);
                }
                chars_written += strlen(arg);
                break;
            }
            case 'u': {
                fmt += 1;
                size_t arg = va_arg(*args, size_t);
                char n_buf[11];
                char *ret = itoa(arg, n_buf, 10);
                if (buf_write) {
                    size_t can_write = (buf_len - 1) - chars_written;
                    if (chars_written > (buf_len - 1)) {
                        can_write = 0;
                    }
                    if (can_write >= strlen(ret)) {
                        memcpy(&buf[chars_written], ret, strlen(ret));
                    } else {
                        memcpy(&buf[chars_written], ret, can_write);
                    }
                } else {
                    puts(ret, serialonly);
                }
                chars_written += strlen(ret);
                break;
            }
            case 'p': {
                fmt += 1;
                uintptr_t arg = va_arg(*args, uintptr_t);
                char n_buf[17];
                char *ret = itoa(arg, n_buf, 16);
                if (buf_write) {
                    size_t can_write = (buf_len - 1) - chars_written;
                    if (chars_written > (buf_len - 1)) {
                        can_write = 0;
                    }
                    if (can_write >= strlen(ret)) {
                        memcpy(&buf[chars_written], ret, strlen(ret));
                    } else {
                        memcpy(&buf[chars_written], ret, can_write);
                    }
                } else {
                    puts(ret, serialonly);
                }
                chars_written += strlen(ret);
                break;
            }
            case 'd': {
                fmt += 1;
                size_t arg = va_arg(*args, size_t);
                char n_buf[12];
                char *ret = itoa_signed(arg, n_buf, 10);
                if (buf_write) {
                    size_t can_write = (buf_len - 1) - chars_written;
                    if (chars_written > (buf_len - 1)) {
                        can_write = 0;
                    }
                    if (can_write >= strlen(ret)) {
                        memcpy(&buf[chars_written], ret, strlen(ret));
                    } else {
                        memcpy(&buf[chars_written], ret, can_write);
                    }
                } else {
                    puts(ret, serialonly);
                }
                chars_written += strlen(ret);
                break;
            }
            case 'c': {
                fmt += 1;
                char arg = va_arg(*args, int);
                if (buf_write) {
                    size_t can_write = (buf_len - 1) - chars_written;
                    if (chars_written > (buf_len - 1)) {
                        can_write = 0;
                    }
                    if (can_write >= 1) {
                        memcpy(&buf[chars_written], &arg, 1);
                    } else {
                        memcpy(&buf[chars_written], &arg, can_write);
                    }
                } else {
                    putc(arg, serialonly);
                }
                chars_written++;
                break;
            }
            default:
                printf("printf: unsupported %%%c\n", *fmt);
                break;
            }
        }
        size_t count = strcspn(fmt, "%");
        if (buf_write) {
            size_t can_write = (buf_len - 1) - chars_written;
            if (chars_written > (buf_len - 1)) {
                can_write = 0;
            }
            if (can_write >= count) {
                memcpy(&buf[chars_written], fmt, count);
            } else {
                memcpy(&buf[chars_written], fmt, can_write);
            }
        } else {
            for (int i = 0; i < count; i++) {
                putc(fmt[i], serialonly);
            }
        }
        chars_written += count;
        fmt += count;
    }
    if (buf_write) {
        if (chars_written < (buf_len - 1)) {
            buf[chars_written] = '\0';
        } else {
            buf[buf_len - 1] = '\0';
        }
    }

    return chars_written;
}

// quick hack for loading linux hello world module
extern "C" int _printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf_base(&args, fmt, nullptr, false, 0, false);
    va_end(args);
    return 0;
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf_base(&args, fmt, nullptr, false, 0, false);
    va_end(args);
}

void printf_serial(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf_base(&args, fmt, nullptr, false, 0, true);
    va_end(args);
}

/*
int snprintf(char *s, size_t n, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = printf_base(&args, fmt, 0, s, true, n);
    va_end(args);
    return ret;
}
*/