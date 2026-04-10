#pragma once
#include <vix/config.h>
#include <vix/drivers/ms_mouse.h>
#include <vix/psf.h>
#include <vix/types.h>

namespace fb {
    struct fbinfo {
        void *address;
        size_t width, height, pitch, bpp;
        bool rgb;
        bool monochrome;
    };

    class fb {
    public:
        fb();
        ~fb();
        void init(struct fbinfo info);
        size_t get_width();
        size_t get_height();
        void write_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b);
        void read_pixel(size_t x, size_t y, uint8_t *r, uint8_t *g, uint8_t *b);
        void flush(); // write changes to screen
        void clear();
        void scroll_y(size_t pixels);

    private:
        struct fbinfo _info;
        void *draw_buffer_address;
#ifdef CONFIG_FB_DOUBLE_BUFFER
        void dirty_write(size_t x, size_t y);
        void dirty_all();
        bool dirty = false;
        // top left
        size_t dirty_a_x = 0;
        size_t dirty_a_y = 0;
        // bottom right
        size_t dirty_b_x = 0;
        size_t dirty_b_y = 0;
#endif
    };

    class fbconsole {
    public:
        void init(fb *framebuffer);
        void init2(); // font init
        void fbputs(const char *str, size_t len);
        void fbputc(char c);

    private:
        fb *_framebuffer;
        void linebreak();
        void putc_screen(char c);
        void parse_esc();

        uint8_t color_foreground[3] = {255, 255, 255}; // rgb
        uint8_t color_background[3] = {0, 0, 0};

        size_t pos_x = 0;
        size_t pos_y = 0;
        psf::psf1_reader psfreader;

        struct termstate {
            bool escaped;
            size_t escapeseq_index;
            char escapeseq[20];
        };
        struct termstate state;
    };
}
