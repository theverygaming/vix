#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

static int printf_base(va_list args, const char *fmt, char *buf, size_t buf_len) {
    size_t chars_written = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case '%': {
                fmt += 1;

                size_t can_write = (buf_len - 1) - chars_written;
                if (chars_written > (buf_len - 1)) {
                    can_write = 0;
                }
                if (can_write >= 1) {
                    memcpy(&buf[chars_written], "%", 1);
                } else {
                    memcpy(&buf[chars_written], "%", can_write);
                }

                chars_written += 1;
                break;
            }
            case 's': {
                fmt += 1;
                char *arg = va_arg(args, char *);

                size_t can_write = (buf_len - 1) - chars_written;
                if (chars_written > (buf_len - 1)) {
                    can_write = 0;
                }
                if (can_write >= strlen(arg)) {
                    memcpy(&buf[chars_written], arg, strlen(arg));
                } else {
                    memcpy(&buf[chars_written], arg, can_write);
                }

                chars_written += strlen(arg);
                break;
            }
            case 'u': {
                fmt += 1;
                size_t arg = va_arg(args, size_t);
                char n_buf[11];
                char *ret = itoa(arg, n_buf, 10);

                size_t can_write = (buf_len - 1) - chars_written;
                if (chars_written > (buf_len - 1)) {
                    can_write = 0;
                }
                if (can_write >= strlen(ret)) {
                    memcpy(&buf[chars_written], ret, strlen(ret));
                } else {
                    memcpy(&buf[chars_written], ret, can_write);
                }

                chars_written += strlen(ret);
                break;
            }
            case 'p': {
                fmt += 1;
                uintptr_t arg = va_arg(args, uintptr_t);
                char n_buf[11];
                char *ret = itoa(arg, n_buf, 16);

                size_t can_write = (buf_len - 1) - chars_written;
                if (chars_written > (buf_len - 1)) {
                    can_write = 0;
                }
                if (can_write >= strlen(ret)) {
                    memcpy(&buf[chars_written], ret, strlen(ret));
                } else {
                    memcpy(&buf[chars_written], ret, can_write);
                }

                chars_written += strlen(ret);
                break;
            }
            case 'd': {
                fmt += 1;
                size_t arg = va_arg(args, size_t);
                char n_buf[12];
                char *ret = itoa_signed(arg, n_buf, 10);

                size_t can_write = (buf_len - 1) - chars_written;
                if (chars_written > (buf_len - 1)) {
                    can_write = 0;
                }
                if (can_write >= strlen(ret)) {
                    memcpy(&buf[chars_written], ret, strlen(ret));
                } else {
                    memcpy(&buf[chars_written], ret, can_write);
                }

                chars_written += strlen(ret);
                break;
            }
            case 'c': {
                fmt += 1;
                char arg = va_arg(args, int);

                size_t can_write = (buf_len - 1) - chars_written;
                if (chars_written > (buf_len - 1)) {
                    can_write = 0;
                }
                if (can_write >= 1) {
                    memcpy(&buf[chars_written], &arg, 1);
                } else {
                    memcpy(&buf[chars_written], &arg, can_write);
                }

                chars_written++;
                break;
            }
            default:
                // unsupported
                goto finish;
                break;
            }
        }
        size_t count = strcspn(fmt, "%");

        size_t can_write = (buf_len - 1) - chars_written;
        if (chars_written > (buf_len - 1)) {
            can_write = 0;
        }
        if (can_write >= count) {
            memcpy(&buf[chars_written], fmt, count);
        } else {
            memcpy(&buf[chars_written], fmt, can_write);
        }

        chars_written += count;
        fmt += count;
    }
finish:
    if (chars_written < (buf_len - 1)) {
        buf[chars_written] = '\0';
    } else {
        buf[buf_len - 1] = '\0';
    }

    return chars_written;
}

int snprintf(char *s, size_t n, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = printf_base(args, fmt, s, n);
    va_end(args);
    return ret;
}

int vsnprintf(char *s, size_t n, const char *fmt, va_list arg) {
    return printf_base(arg, fmt, s, n);
}

#ifdef __cplusplus
}
#endif
