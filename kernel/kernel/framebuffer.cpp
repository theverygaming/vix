#include <debug.h>
#include <framebuffer.h>
#include <fs/vfs.h>
#include <panic.h>
#include <stdlib.h>

void fb::fb::init(struct fbinfo info) {
    _info = info;
}
size_t fb::fb::get_width() {
    return _info.width;
}
size_t fb::fb::get_height() {
    return _info.height;
}

void fb::fb::write_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b) {
    size_t offset = _info.pitch * y + (_info.bpp / 8) * x;
    *(((uint8_t *)_info.address) + offset + 0) = b;
    *(((uint8_t *)_info.address) + offset + 1) = g;
    *(((uint8_t *)_info.address) + offset + 2) = r;
}

void fb::fb::clear() {
    memset(_info.address, 0, _info.pitch * _info.height);
}

void fb::fbconsole::init(fb *framebuffer) {
    _framebuffer = framebuffer;
    _framebuffer->clear();
}

void fb::fbconsole::init2() {
    void *fontptr;
    if (fs::vfs::fptr("/ramfs/Unifont-APL8x16-15.0.01.psf", &fontptr)) {
        if (!psfreader.init(fontptr)) {
            KERNEL_PANIC("could not initialize font"); // this stupid
        }
    } else {
        KERNEL_PANIC("could not find font");
    }
}

void fb::fbconsole::puts(char *str, arch::generic::textoutput::color color) {
    for (int i = 0; i < 3; i++) {
        currentcolor[i] = 255;
    }
    while (*str) {
        this->putc(*str);
        str++;
    }
}

#define FONT_X_SPACING 0
#define FONT_Y_SPACING 0

void fb::fbconsole::linebreak() {
    if ((pos_y + psfreader.get_height() + FONT_Y_SPACING + psfreader.get_height()) < _framebuffer->get_height()) {
        pos_y += psfreader.get_height() + FONT_Y_SPACING;
    } else {
        pos_y = 0;
        _framebuffer->clear();
    }
    pos_x = 0;
}

static inline uint8_t bitget(uint32_t num, uint8_t bit) {
    return (num & (0x1 << bit)) >> bit;
}

void fb::fbconsole::putc(char c) {
    for (int i = 0; i < 3; i++) {
        currentcolor[i] = 255;
    }
    // special characters
    switch (c) {
    case '\n':
        linebreak();
        return;
        break;
    case '\t':
        for (int i = 0; i < 4; i++) {
            this->putc(' ');
        }
        return;
        break;
    case '\r':
        return;
        break;
    default:
        break;
    }

    int count = 0;
    char fontbuf[(psfreader.get_width() * psfreader.get_height()) / 8]; // hacky af
    psfreader.get_glyph(fontbuf, c);
    for (int y = pos_y; y < pos_y + psfreader.get_height(); y++) {
        for (int x = pos_x; x < pos_x + psfreader.get_width(); x++) {
            size_t bufindex = count / 8;
            size_t offset = count % 8;
            offset = 7 - offset; // invert offset
            if (bitget(fontbuf[bufindex], offset)) {
                _framebuffer->write_pixel(x - 1, y, 255, 255, 255);
            } else {
                _framebuffer->write_pixel(x - 1, y, 0, 0, 0);
            }
            count++;
        }
    }
    if ((pos_x + psfreader.get_width() + FONT_X_SPACING + psfreader.get_width()) < _framebuffer->get_width()) {
        pos_x += psfreader.get_width() + FONT_X_SPACING;
    } else {
        linebreak();
    }
}
