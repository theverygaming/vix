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

char *itoa(size_t value, char *str, size_t base) {
    char *ptr = str;

    do {
        size_t mod = value % base;
        unsigned char start = '0';
        if ((base == 16) && (mod > 9)) {
            start = 'a';
            mod -= 10;
        }
        *ptr++ = start + mod;
    } while ((value /= base) > 0);
    *ptr = '\0';

    size_t len = strlen(str);

    for (int i = 0; i < len / 2; i++) {
        char c = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = c;
    }

    return str;
}

char *itoa_signed(ssize_t value, char *str, size_t base) {
    bool sign = false;
    if (value < 0) {
        sign = true;
        value = -value;
    }

    char *ptr = str;

    int dig = 0;
    do {
        size_t mod = value % base;
        unsigned char start = '0';
        if ((base == 16) && (mod > 9)) {
            start = 'a';
            mod -= 10;
        }
        *ptr++ = start + mod;
    } while ((value /= base) > 0);
    if (sign) {
        *ptr++ = '-';
    }
    *ptr = '\0';

    size_t len = strlen(str);

    for (int i = 0; i < len / 2; i++) {
        char c = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = c;
    }

    return str;
}

char *strchr(char *str, int character) {
    while (*str) {
        if (*str == character) {
            return str;
        }
        str++;
    }
    if (*str == character) {
        return str;
    }
    return 0;
}

size_t strcspn(const char *s1, const char *s2) {
    size_t n = 0;
    if (*s2 == 0) {
        return 0;
    }
    while (*s1) {
        if (strchr((char *)s2, *s1)) {
            return n;
        }
        s1++;
        n++;
    }
    return n;
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

int snprintf(char *s, size_t n, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = printf_base(&args, fmt, 0, s, true, n);
    va_end(args);
    return ret;
}
