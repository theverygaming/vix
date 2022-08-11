#include <arch/x86/cpubasics.h>
#include "types.h"

#include "../config.h"
#include <arch/x86/drivers/serial.h>
#include <cstdarg>
#include <stdbool.h>

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const uint8_t DEFAULT_COLOR = 0x7;

uint8_t *g_ScreenBuffer = (uint8_t *)(KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET);
int g_ScreenX = 0, g_ScreenY = 0;

void putchr(int x, int y, char c) {
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)] = c;
}

void putcolor(int x, int y, uint8_t color) {
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1] = color;
}

char getchr(int x, int y) {
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)];
}

uint8_t getcolor(int x, int y) {
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1];
}

void setcursor(int x, int y) {
    int pos = y * SCREEN_WIDTH + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void clrscr() {
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            putchr(x, y, '\0');
            putcolor(x, y, DEFAULT_COLOR);
        }

    g_ScreenX = 0;
    g_ScreenY = 0;
    setcursor(g_ScreenX, g_ScreenY);
}

void scrollback(int lines) {
    for (int y = lines; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            putchr(x, y - lines, getchr(x, y));
            putcolor(x, y - lines, getcolor(x, y));
        }
    }

    for (int y = SCREEN_HEIGHT - lines; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            putchr(x, y, '\0');
            putcolor(x, y, DEFAULT_COLOR);
        }
    }

    g_ScreenY -= lines;
}

void putc(char c, bool serialonly) {
    drivers::serial::putc(c);
    if (serialonly) {
        return;
    }
    switch (c) {
    case '\n':
        g_ScreenX = 0;
        g_ScreenY++;
        break;

    case '\t':
        for (int i = 0; i < 4 - (g_ScreenX % 4); i++)
            putc(' ', false);
        break;

    case '\r':
        g_ScreenX = 0;
        break;

    default:
        putchr(g_ScreenX, g_ScreenY, c);
        g_ScreenX++;
        break;
    }

    if (g_ScreenX >= SCREEN_WIDTH) {
        g_ScreenY++;
        g_ScreenX = 0;
    }
    if (g_ScreenY >= SCREEN_HEIGHT)
        scrollback(1);

    setcursor(g_ScreenX, g_ScreenY);
}

void puts(const char *str, bool serialonly) {
    while (*str) {
        putc(*str, serialonly);
        str++;
    }
}

const char g_HexChars[] = "0123456789abcdef";

void printf_unsigned(uint32_t number, int radix, bool serialonly) // long long causes issue so using 32 bit unsigned
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
        putc(buffer[pos], serialonly);
}

void printf_signed(long long number, int radix, bool serialonly) {
    if (number < 0) {
        putc('-', serialonly);
        printf_unsigned(-number, radix, serialonly);
    } else
        printf_unsigned(number, radix, serialonly);
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

void printf_core(bool serialonly, va_list args, const char *fmt, ...) {
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
                putc(*fmt, serialonly);
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
                putc((char)va_arg(args, int), serialonly);
                break;

            case 's':
                puts(va_arg(args, const char *), serialonly);
                break;

            case '%':
                putc('%', serialonly);
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
                        printf_signed(va_arg(args, int), radix, serialonly);
                        break;

                    case PRINTF_LENGTH_LONG:
                        printf_signed(va_arg(args, long), radix, serialonly);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:
                        printf_signed(va_arg(args, long long), radix, serialonly);
                        break;
                    }
                } else {
                    switch (length) {
                    case PRINTF_LENGTH_SHORT_SHORT:
                    case PRINTF_LENGTH_SHORT:
                    case PRINTF_LENGTH_DEFAULT:
                        printf_unsigned(va_arg(args, unsigned int), radix, serialonly);
                        break;

                    case PRINTF_LENGTH_LONG:
                        printf_unsigned(va_arg(args, unsigned long), radix, serialonly);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:
                        printf_unsigned(va_arg(args, unsigned long long), radix, serialonly);
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