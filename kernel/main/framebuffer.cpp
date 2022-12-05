#include <framebuffer.h>
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

void fb::fbconsole::puts(char *str, arch::generic::textoutput::color color) {
    for (int i = 0; i < 3; i++) {
        currentcolor[i] = 255;
    }
    while (*str) {
        this->putc(*str);
        str++;
    }
}

void fb::fbconsole::linebreak() {
    if ((pos_y + 7 + 6) < _framebuffer->get_height()) {
        pos_y += 7;
    } else {
        pos_y = 0;
        _framebuffer->clear();
    }
    pos_x = 0;
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
    if (c >= 'a' && c <= 'z') {
        c -= 32;
    }
    if (c > 'Z') {
        return;
    }

    // font stuff from my old OS
    int count = 0;
    for (int y = pos_y; y < pos_y + 6; y++) {
        for (int x = pos_x; x < pos_x + 6; x++) {
            count++;
        }
    }
    if ((pos_x + 7 + 6) < _framebuffer->get_width()) {
        pos_x += 7;
    } else {
        linebreak();
    }
}
