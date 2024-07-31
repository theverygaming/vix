#include <vix/arch/cpubasics.h>
#include <vix/arch/drivers/text80x25.h>
#include <vix/config.h>

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

static const uint8_t DEFAULT_COLOR = 0x7;

static uint8_t *g_ScreenBuffer = (uint8_t *)(KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET);
static int g_ScreenX = 0, g_ScreenY = 0;

static void putchr(int x, int y, char c) {
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)] = c;
}

static char getchr(int x, int y) {
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)];
}

void putcolor(int x, int y, uint8_t color) { // TODO: make this more universal
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1] = color;
}

static uint8_t getcolor(int x, int y) {
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1];
}

static void setcursor(int x, int y) {
    int pos = y * SCREEN_WIDTH + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void clrscr() {
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            putchr(x, y, '\0');
            putcolor(x, y, DEFAULT_COLOR);
        }

    g_ScreenX = 0;
    g_ScreenY = 0;
    setcursor(g_ScreenX, g_ScreenY);
}

static void scrollback(int lines) {
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

void drivers::textmode::text80x25::init() {
    clrscr();
}

void drivers::textmode::text80x25::putc(char c, color foreground, color background) {
    uint8_t color = ((uint8_t)foreground & 0xF) | ((uint8_t)background << 4);
    switch (c) {
    case '\n':
        g_ScreenX = 0;
        g_ScreenY++;
        break;

    case '\t':
        for (int i = 0; i < 4 - (g_ScreenX % 4); i++)
            putc(' ', foreground, background);
        break;

    case '\r':
        g_ScreenX = 0;
        break;

    default:
        putcolor(g_ScreenX, g_ScreenY, color);
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

void drivers::textmode::text80x25::putc(char c) {
    putc(c, COLOR_GREY, COLOR_BLACK);
}

void drivers::textmode::text80x25::delc() {
    if (g_ScreenX == 0) {
        if (g_ScreenY > 0) {
            g_ScreenY -= 1;
        }
        g_ScreenX = SCREEN_WIDTH - 1;
    } else if (g_ScreenX < SCREEN_WIDTH) {
        g_ScreenX--;
    }
    putchr(g_ScreenX, g_ScreenY, '\0');
    putcolor(g_ScreenX, g_ScreenY, DEFAULT_COLOR);
    setcursor(g_ScreenX, g_ScreenY);
}
