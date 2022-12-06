#pragma once
#include <arch/arch.h>
#include <psf.h>
#include <types.h>
#include INCLUDE_ARCH_GENERIC(textoutput.h)

namespace fb {
    struct fbinfo {
        void *address;
        size_t width, height, pitch, bpp;
    };

    class fb {
    public:
        void init(struct fbinfo info);
        size_t get_width();
        size_t get_height();
        void write_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b);
        void clear();

    private:
        struct fbinfo _info;
    };

    class fbconsole {
    public:
        void init(fb *framebuffer);
        void init2(); // font init
        void puts(char *str, arch::generic::textoutput::color color = arch::generic::textoutput::color::COLOR_WHITE);
        void putc(char c);

    private:
        fb *_framebuffer;
        void linebreak();
        uint8_t currentcolor[3]; // rgb

        size_t pos_x = 0;
        size_t pos_y = 0;
        psf::psf1_reader psfreader;
    };
}
