#include <psf.h>
#include <stdlib.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

struct __attribute__((packed)) psf1_header {
    uint8_t magic[2];
    uint8_t mode;       // non-unicode: {0: 256 chars 1: 512 chars} unicode: {2: 256 chars 3: 512 chars}
    uint8_t charheight; // char height (font is 8xcharheight)
};

#include <debug.h>

bool psf::psf1_reader::init(void *fontptr) {
    _fontptr = fontptr;
    _init = false;
    width = 0;
    height = 0;
    struct psf1_header *header = (struct psf1_header *)_fontptr;
    if (header->magic[0] != PSF1_MAGIC0 || header->magic[1] != PSF1_MAGIC1) {
        return false;
    }
    width = 8;
    height = header->charheight;
    _init = true;
    return _init;
}

size_t psf::psf1_reader::get_width() {
    return width;
}

size_t psf::psf1_reader::get_height() {
    return height;
}

void psf::psf1_reader::get_glyph(void *ptr, uint16_t character) {
    struct psf1_header *header = (struct psf1_header *)_fontptr;
    if ((character > 511) || (character > 255 && (header->mode == 0 || header->mode == 2))) {
        return;
    }
    memcpy(ptr, ((char *)_fontptr) + sizeof(struct psf1_header) + (character * height), height);
}
