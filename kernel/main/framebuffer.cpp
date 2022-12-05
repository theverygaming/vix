#include <framebuffer.h>

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
