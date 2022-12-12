#include <arch/drivers/serial.h>
#include <arch/drivers/text80x25.h>
#include <arch/generic/textoutput.h>

static inline drivers::textmode::text80x25::color translatecolor(arch::generic::textoutput::color in) {
    switch (in) {
    case arch::generic::textoutput::color::COLOR_GREY:
        return drivers::textmode::text80x25::COLOR_GREY;
    case arch::generic::textoutput::color::COLOR_BLACK:
        return drivers::textmode::text80x25::COLOR_BLACK;
    case arch::generic::textoutput::color::COLOR_WHITE:
        return drivers::textmode::text80x25::COLOR_WHITE;
    case arch::generic::textoutput::color::COLOR_RED:
        return drivers::textmode::text80x25::COLOR_RED;
    case arch::generic::textoutput::color::COLOR_LIGHT_RED:
        return drivers::textmode::text80x25::COLOR_LIGHT_RED;
    case arch::generic::textoutput::color::COLOR_GREEN:
        return drivers::textmode::text80x25::COLOR_GREEN;
    case arch::generic::textoutput::color::COLOR_LIGHT_GREEN:
        return drivers::textmode::text80x25::COLOR_LIGHT_GREEN;
    case arch::generic::textoutput::color::COLOR_BLUE:
        return drivers::textmode::text80x25::COLOR_BLUE;
    case arch::generic::textoutput::color::COLOR_LIGHT_BLUE:
        return drivers::textmode::text80x25::COLOR_LIGHT_BLUE;
    }
    return drivers::textmode::text80x25::COLOR_RED;
}

void arch::generic::textoutput::puts(const char *str, color foreground, color background) {
    // oh god fuck horibel
    drivers::textmode::text80x25::color foreground_txt = translatecolor(foreground);
    drivers::textmode::text80x25::color background_txt = translatecolor(background);

    while (*str) {
        drivers::textmode::text80x25::putc(*str, foreground_txt, background_txt);
        drivers::serial::putc(*str);
        str++;
    }
}
