#pragma once
#include <types.h>

namespace psf {
    class psf1_reader {
    public:
        bool init(void *fontptr);

        size_t get_width();
        size_t get_height();

        void get_glyph(void *ptr, uint16_t character);

    private:
        void *_fontptr;
        bool _init = false;
        size_t width = 0;
        size_t height = 0;
    };
}
