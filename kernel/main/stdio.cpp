#include <config.h>
#include <stdio.h>
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

const char g_HexChars[] = "0123456789abcdef";

void printf_unsigned(uint32_t number, int radix, bool debugonly) // long long causes issue so using 32 bit unsigned
{
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        putc(buffer[pos], debugonly);
}

void printf_signed(long long number, int radix, bool debugonly) {
    if (number < 0) {
        putc('-', debugonly);
        printf_unsigned(-number, radix, debugonly);
    } else
        printf_unsigned(number, radix, debugonly);
}

#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG 3
#define PRINTF_STATE_SPEC 4

#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

void printf_core(bool debugonly, va_list args, const char *fmt, ...) {
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt) {
        switch (state) {
        case PRINTF_STATE_NORMAL:
            switch (*fmt) {
            case '%':
                state = PRINTF_STATE_LENGTH;
                break;
            default:
                putc(*fmt, debugonly);
                break;
            }
            break;

        case PRINTF_STATE_LENGTH:
            switch (*fmt) {
            case 'h':
                length = PRINTF_LENGTH_SHORT;
                state = PRINTF_STATE_LENGTH_SHORT;
                break;
            case 'l':
                length = PRINTF_LENGTH_LONG;
                state = PRINTF_STATE_LENGTH_LONG;
                break;
            default:
                goto PRINTF_STATE_SPEC_;
            }
            break;

        case PRINTF_STATE_LENGTH_SHORT:
            if (*fmt == 'h') {
                length = PRINTF_LENGTH_SHORT_SHORT;
                state = PRINTF_STATE_SPEC;
            } else
                goto PRINTF_STATE_SPEC_;
            break;

        case PRINTF_STATE_LENGTH_LONG:
            if (*fmt == 'l') {
                length = PRINTF_LENGTH_LONG_LONG;
                state = PRINTF_STATE_SPEC;
            } else
                goto PRINTF_STATE_SPEC_;
            break;

        case PRINTF_STATE_SPEC:
        PRINTF_STATE_SPEC_:
            switch (*fmt) {
            case 'c':
                putc((char)va_arg(args, int), debugonly);
                break;

            case 's':
                puts(va_arg(args, const char *), debugonly);
                break;

            case '%':
                putc('%', debugonly);
                break;

            case 'd':
            case 'i':
                radix = 10;
                sign = true;
                number = true;
                break;

            case 'u':
                radix = 10;
                sign = false;
                number = true;
                break;

            case 'X':
            case 'x':
            case 'p':
                radix = 16;
                sign = false;
                number = true;
                break;

            case 'o':
                radix = 8;
                sign = false;
                number = true;
                break;

            // ignore invalid spec
            default:
                break;
            }

            if (number) {
                if (sign) {
                    switch (length) {
                    case PRINTF_LENGTH_SHORT_SHORT:
                    case PRINTF_LENGTH_SHORT:
                    case PRINTF_LENGTH_DEFAULT:
                        printf_signed(va_arg(args, int), radix, debugonly);
                        break;

                    case PRINTF_LENGTH_LONG:
                        printf_signed(va_arg(args, long), radix, debugonly);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:
                        printf_signed(va_arg(args, long long), radix, debugonly);
                        break;
                    }
                } else {
                    switch (length) {
                    case PRINTF_LENGTH_SHORT_SHORT:
                    case PRINTF_LENGTH_SHORT:
                    case PRINTF_LENGTH_DEFAULT:
                        printf_unsigned(va_arg(args, unsigned int), radix, debugonly);
                        break;

                    case PRINTF_LENGTH_LONG:
                        printf_unsigned(va_arg(args, unsigned long), radix, debugonly);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:
                        printf_unsigned(va_arg(args, unsigned long long), radix, debugonly);
                        break;
                    }
                }
            }

            // reset state
            state = PRINTF_STATE_NORMAL;
            length = PRINTF_LENGTH_DEFAULT;
            radix = 10;
            sign = false;
            number = false;
            break;
        }

        fmt++;
    }
}

// quick hack for loading linux hello world module
extern "C" int _printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf_core(false, args, fmt);
    va_end(args);
    return 0;
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf_core(false, args, fmt);
    va_end(args);
}

void printf_serial(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf_core(true, args, fmt);
    va_end(args);
}

void print_buffer(const char *msg, const void *buffer, uint32_t count) {
    const uint8_t *u8Buffer = (const uint8_t *)buffer;

    puts(msg, false);
    for (uint16_t i = 0; i < count; i++) {
        putc(g_HexChars[u8Buffer[i] >> 4], false);
        putc(g_HexChars[u8Buffer[i] & 0xF], false);
    }
    puts("\n", false);
}
