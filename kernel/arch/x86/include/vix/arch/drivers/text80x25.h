#pragma once
#include <vix/types.h>

namespace drivers::textmode::text80x25 {
    enum color {
        COLOR_BLACK = 0,
        COLOR_BLUE = 1,
        COLOR_GREEN = 2,
        COLOR_CYAN = 3,
        COLOR_RED = 4,
        COLOR_PURPLE = 5,
        COLOR_BROWN = 6,
        COLOR_GREY = 7,
        COLOR_DARK_GREY = 8,
        COLOR_LIGHT_BLUE = 9,
        COLOR_LIGHT_GREEN = 10,
        COLOR_LIGHT_CYAN = 11,
        COLOR_LIGHT_RED = 12,
        COLOR_LIGHT_PURPLE = 13,
        COLOR_YELLOW = 14,
        COLOR_WHITE = 15,
    };
    void init();
    void putc(char c, color foreground = COLOR_GREY, color background = COLOR_BLACK);
    void putc(char c);
    void delc();
}
