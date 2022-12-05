#pragma once
#include <arch/x86/multiboot2.h>
#include <types.h>

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

    private:
        struct fbinfo _info;
    };
}
