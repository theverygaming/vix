#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <types.h>

static char kp_buf[CONFIG_KPRINTF_BUFSIZE];
static size_t kp_buf_position = 0;

inline void putc_kbuf(char c) {
    putc(c, true);
}

inline void puts_kbuf(char *s) {
    while (*s) {
        putc_kbuf(*s++);
    }
}

static int kprintf_base(va_list *args, const char *fmt) {
    size_t chars_written = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case '%': {
                fmt += 1;
                putc_kbuf('%');
                chars_written += 1;
                break;
            }
            case 's': {
                fmt += 1;
                char *arg = va_arg(*args, char *);
                puts_kbuf(arg);
                chars_written += strlen(arg);
                break;
            }
            case 'u': {
                fmt += 1;
                size_t arg = va_arg(*args, size_t);
                char n_buf[11];
                char *ret = itoa(arg, n_buf, 10);
                puts_kbuf(ret);
                chars_written += strlen(ret);
                break;
            }
            case 'p': {
                fmt += 1;
                uintptr_t arg = va_arg(*args, uintptr_t);
                char n_buf[17];
                char *ret = itoa(arg, n_buf, 16);
                puts_kbuf(ret);
                chars_written += strlen(ret);
                break;
            }
            case 'd': {
                fmt += 1;
                ssize_t arg = va_arg(*args, ssize_t);
                char n_buf[12];
                char *ret = itoa_signed(arg, n_buf, 10);
                puts_kbuf(ret);
                chars_written += strlen(ret);
                break;
            }
            case 'c': {
                fmt += 1;
                char arg = va_arg(*args, int);
                putc_kbuf(arg);
                chars_written++;
                break;
            }
            default:
                puts_kbuf("kprintf: unsupported %");
                putc_kbuf(*fmt);
                break;
            }
        }
        size_t count = strcspn(fmt, "%");
        for (int i = 0; i < count; i++) {
            putc_kbuf(fmt[i]);
        }
        chars_written += count;
        fmt += count;
    }

    return chars_written;
}

static void kprintf_internal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kprintf_base(&args, fmt);
    va_end(args);
}

static int current_loglevel = CONFIG_KPRINTF_LOGLEVEL;

void kprintf(int loglevel, const char *fmt, ...) {
    if (loglevel <= current_loglevel) {
        kprintf_internal("<%d>[%u] ", loglevel, (size_t)time::ms_since_bootup / 1000);
        va_list args;
        va_start(args, fmt);
        kprintf_base(&args, fmt);
        va_end(args);
    }
}

void kp_debug(const char *fmt, ...) {
    if (KP_DEBUG <= current_loglevel) {
        kprintf_internal("<%d>[%u] ", (int)KP_DEBUG, (size_t)time::ms_since_bootup / 1000);
        va_list args;
        va_start(args, fmt);
        kprintf_base(&args, fmt);
        va_end(args);
    }
}
