#include <config.h>
#include <kprintf.h>
#include <macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <types.h>

static char kp_buf[CONFIG_KPRINTF_BUFSIZE];
static size_t kp_buf_position = 0;

inline void putc_kbuf(char c, int loglevel) {
    if (unlikely(loglevel <= KP_ALERT)) {
        putc(c, false);
        return; // FIXME: this is a workaround for double printing
    }
    putc(c, true);
}

inline void puts_kbuf(char *s, int loglevel) {
    while (*s) {
        putc_kbuf(*s++, loglevel);
    }
}

static int kprintf_base(va_list *args, const char *fmt, int loglevel) {
    size_t chars_written = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case '%': {
                fmt += 1;
                putc_kbuf('%', loglevel);
                chars_written += 1;
                break;
            }
            case 's': {
                fmt += 1;
                char *arg = va_arg(*args, char *);
                puts_kbuf(arg, loglevel);
                chars_written += strlen(arg);
                break;
            }
            case 'u': {
                fmt += 1;
                size_t arg = va_arg(*args, size_t);
                char n_buf[11];
                char *ret = itoa(arg, n_buf, 10);
                puts_kbuf(ret, loglevel);
                chars_written += strlen(ret);
                break;
            }
            case 'p': {
                fmt += 1;
                uintptr_t arg = va_arg(*args, uintptr_t);
                char n_buf[17];
                char *ret = itoa(arg, n_buf, 16);
                puts_kbuf(ret, loglevel);
                chars_written += strlen(ret);
                break;
            }
            case 'd': {
                fmt += 1;
                ssize_t arg = va_arg(*args, ssize_t);
                char n_buf[12];
                char *ret = itoa_signed(arg, n_buf, 10);
                puts_kbuf(ret, loglevel);
                chars_written += strlen(ret);
                break;
            }
            case 'c': {
                fmt += 1;
                char arg = va_arg(*args, int);
                putc_kbuf(arg, loglevel);
                chars_written++;
                break;
            }
            default:
                puts_kbuf("[kprintf: unsupported %", loglevel);
                putc_kbuf(*fmt, loglevel);
                putc(']', loglevel);
                break;
            }
        }
        size_t count = strcspn(fmt, "%");
        for (size_t i = 0; i < count; i++) {
            putc_kbuf(fmt[i], loglevel);
        }
        chars_written += count;
        fmt += count;
    }

    return chars_written;
}

static size_t log10(size_t n) {
    size_t r = 0;
    while (n >= 10) {
        n /= 10;
        r++;
    }
    return r;
}

static void kprintf_internal(int loglevel, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kprintf_base(&args, fmt, loglevel);
    va_end(args);
}

static int current_loglevel = CONFIG_KPRINTF_LOGLEVEL;

void kprintf(int loglevel, const char *fmt, ...) {
    if (loglevel <= current_loglevel) {
        // shift results in precision loss but it's fast
        size_t secs = (size_t)(time::ns_since_bootup >> 20) / 1000;
        size_t ms = (size_t)(time::ns_since_bootup >> 20) % 1000;
        char zeros[] = "00";
        zeros[2 - log10(ms)] = '\0';
        kprintf_internal(loglevel, "<%d>[%u.%s%u] ", loglevel, secs, zeros, ms);
        va_list args;
        va_start(args, fmt);
        kprintf_base(&args, fmt, loglevel);
        va_end(args);
    }
}
