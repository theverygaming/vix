#include <cppstd/algorithm.h>
#include <debug.h>
#include <drivers/ms_mouse.h>
#include <framebuffer.h>
#include <fs/vfs.h>
#include <panic.h>
#include <stdlib.h>

static inline uint8_t bitget(uint32_t num, uint8_t bit) {
    return (num & (0x1 << bit)) >> bit;
}

constexpr size_t mouse_w = 8;
constexpr size_t mouse_h = 8;

uint8_t cursor[8] = {
    0b01111111,
    0b00000011,
    0b00000101,
    0b00001001,
    0b00010001,
    0b00100001,
    0b01000001,
    0b10000000,
};

static ssize_t mouse_x = 0;
static ssize_t mouse_y = 0;

bool restorebg = false;
uint8_t mouse_bg[mouse_w * mouse_h * 3];

void fb::fb::mouse_event(void *self, struct drivers::ms_mouse::mouse_packet packet) {
    fb *_self = (fb *)self;
    if (restorebg && mouse_x + mouse_w < _self->_info.width && mouse_y + mouse_h < _self->_info.height) {
        for (int y = 0; y < mouse_h; y++) {
            for (int x = 0; x < mouse_w; x++) {
                int index_o = (y * mouse_w * 3) + (x * 3);
                _self->write_pixel(mouse_x + x, mouse_y + y, mouse_bg[index_o], mouse_bg[index_o + 1], mouse_bg[index_o + 2]);
            }
        }
    }

    mouse_x += packet.x_movement;
    mouse_y -= packet.y_movement; // inverted for Y

    mouse_x = std::clamp(mouse_x, (ssize_t)0, (ssize_t)_self->_info.width);
    mouse_y = std::clamp(mouse_y, (ssize_t)0, (ssize_t)_self->_info.height);

    if (mouse_x + mouse_w < _self->_info.width && mouse_y + mouse_h < _self->_info.height) {
        for (int y = 0; y < mouse_h; y++) {
            for (int x = 0; x < mouse_w; x++) {
                int index_o = (y * mouse_w * 3) + (x * 3);
                _self->read_pixel(mouse_x + x, mouse_y + y, &mouse_bg[index_o], &mouse_bg[index_o + 1], &mouse_bg[index_o + 2]);
                if (bitget(cursor[y], x)) {
                    _self->write_pixel(mouse_x + x, mouse_y + y, 255, 255, 255);
                }
            }
        }
        restorebg = true;
    }
}

fb::fb::fb() {
    drivers::ms_mouse::register_event_listener(mouse_event, this);
}

fb::fb::~fb() {
    drivers::ms_mouse::deregister_event_listener(mouse_event, this);
}

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

void fb::fb::read_pixel(size_t x, size_t y, uint8_t *r, uint8_t *g, uint8_t *b) {
    size_t offset = _info.pitch * y + (_info.bpp / 8) * x;
    *r = *(((uint8_t *)_info.address) + offset + 0);
    *g = *(((uint8_t *)_info.address) + offset + 1);
    *b = *(((uint8_t *)_info.address) + offset + 2);
}

void fb::fb::clear() {
    memset(_info.address, 0, _info.pitch * _info.height);
}

void fb::fb::scroll_y(size_t pixels) {
    size_t move_bytes = _info.pitch * pixels;
    size_t all_bytes = _info.pitch * _info.height;
    memmove(_info.address, ((uint8_t *)_info.address) + move_bytes, all_bytes - move_bytes);
    memset(((uint8_t *)_info.address) + (all_bytes - move_bytes), 0, move_bytes);
}

void fb::fbconsole::init(fb *framebuffer) {
    _framebuffer = framebuffer;
    _framebuffer->clear();

    state.escaped = false;
    state.escapeseq_index = 0;
    pos_x = 0;
    pos_y = 0;
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

void fb::fbconsole::puts(char *str) {
    while (*str) {
        this->putc(*str);
        str++;
    }
}

void fb::fbconsole::linebreak() {
    if ((pos_y + psfreader.get_height() + psfreader.get_height()) < _framebuffer->get_height()) {
        pos_y += psfreader.get_height();
    } else {
        _framebuffer->scroll_y(psfreader.get_height());
    }
    pos_x = 0;
}

static int countc(const char *str, char c) {
    int count = 0;
    while (*str) {
        if (*str++ == c) {
            count++;
        }
    }
    return count;
}

// https://github.com/limine-bootloader/terminal/blob/trunk/term.c
static const uint32_t col256[] = {
    0x000000, 0x00005f, 0x000087, 0x0000af, 0x0000d7, 0x0000ff, 0x005f00, 0x005f5f, 0x005f87, 0x005faf, 0x005fd7, 0x005fff, 0x008700, 0x00875f, 0x008787, 0x0087af, 0x0087d7, 0x0087ff, 0x00af00,
    0x00af5f, 0x00af87, 0x00afaf, 0x00afd7, 0x00afff, 0x00d700, 0x00d75f, 0x00d787, 0x00d7af, 0x00d7d7, 0x00d7ff, 0x00ff00, 0x00ff5f, 0x00ff87, 0x00ffaf, 0x00ffd7, 0x00ffff, 0x5f0000, 0x5f005f,
    0x5f0087, 0x5f00af, 0x5f00d7, 0x5f00ff, 0x5f5f00, 0x5f5f5f, 0x5f5f87, 0x5f5faf, 0x5f5fd7, 0x5f5fff, 0x5f8700, 0x5f875f, 0x5f8787, 0x5f87af, 0x5f87d7, 0x5f87ff, 0x5faf00, 0x5faf5f, 0x5faf87,
    0x5fafaf, 0x5fafd7, 0x5fafff, 0x5fd700, 0x5fd75f, 0x5fd787, 0x5fd7af, 0x5fd7d7, 0x5fd7ff, 0x5fff00, 0x5fff5f, 0x5fff87, 0x5fffaf, 0x5fffd7, 0x5fffff, 0x870000, 0x87005f, 0x870087, 0x8700af,
    0x8700d7, 0x8700ff, 0x875f00, 0x875f5f, 0x875f87, 0x875faf, 0x875fd7, 0x875fff, 0x878700, 0x87875f, 0x878787, 0x8787af, 0x8787d7, 0x8787ff, 0x87af00, 0x87af5f, 0x87af87, 0x87afaf, 0x87afd7,
    0x87afff, 0x87d700, 0x87d75f, 0x87d787, 0x87d7af, 0x87d7d7, 0x87d7ff, 0x87ff00, 0x87ff5f, 0x87ff87, 0x87ffaf, 0x87ffd7, 0x87ffff, 0xaf0000, 0xaf005f, 0xaf0087, 0xaf00af, 0xaf00d7, 0xaf00ff,
    0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fd7, 0xaf5fff, 0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af, 0xaf87d7, 0xaf87ff, 0xafaf00, 0xafaf5f, 0xafaf87, 0xafafaf, 0xafafd7, 0xafafff, 0xafd700,
    0xafd75f, 0xafd787, 0xafd7af, 0xafd7d7, 0xafd7ff, 0xafff00, 0xafff5f, 0xafff87, 0xafffaf, 0xafffd7, 0xafffff, 0xd70000, 0xd7005f, 0xd70087, 0xd700af, 0xd700d7, 0xd700ff, 0xd75f00, 0xd75f5f,
    0xd75f87, 0xd75faf, 0xd75fd7, 0xd75fff, 0xd78700, 0xd7875f, 0xd78787, 0xd787af, 0xd787d7, 0xd787ff, 0xd7af00, 0xd7af5f, 0xd7af87, 0xd7afaf, 0xd7afd7, 0xd7afff, 0xd7d700, 0xd7d75f, 0xd7d787,
    0xd7d7af, 0xd7d7d7, 0xd7d7ff, 0xd7ff00, 0xd7ff5f, 0xd7ff87, 0xd7ffaf, 0xd7ffd7, 0xd7ffff, 0xff0000, 0xff005f, 0xff0087, 0xff00af, 0xff00d7, 0xff00ff, 0xff5f00, 0xff5f5f, 0xff5f87, 0xff5faf,
    0xff5fd7, 0xff5fff, 0xff8700, 0xff875f, 0xff8787, 0xff87af, 0xff87d7, 0xff87ff, 0xffaf00, 0xffaf5f, 0xffaf87, 0xffafaf, 0xffafd7, 0xffafff, 0xffd700, 0xffd75f, 0xffd787, 0xffd7af, 0xffd7d7,
    0xffd7ff, 0xffff00, 0xffff5f, 0xffff87, 0xffffaf, 0xffffd7, 0xffffff, 0x080808, 0x121212, 0x1c1c1c, 0x262626, 0x303030, 0x3a3a3a, 0x444444, 0x4e4e4e, 0x585858, 0x626262, 0x6c6c6c, 0x767676,
    0x808080, 0x8a8a8a, 0x949494, 0x9e9e9e, 0xa8a8a8, 0xb2b2b2, 0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada, 0xe4e4e4, 0xeeeeee};

// https://github.com/limine-bootloader/terminal/blob/trunk/backends/framebuffer.c

static const uint32_t col_ansi[]{
    0x00000000,
    0x00aa0000,
    0x0000aa00,
    0x00aa5500,
    0x000000aa,
    0x00aa00aa,
    0x0000aaaa,
    0x00aaaaaa,
    0x00555555,
    0x00ff5555,
    0x0055ff55,
    0x00ffff55,
    0x005555ff,
    0x00ff55ff,
    0x0055ffff,
    0x00ffffff,
};

void fb::fbconsole::parse_esc() {
    // reference: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
    if (state.escapeseq[0] != 0x1b || state.escapeseq[1] != 0x0 || state.escapeseq[2] != 0x5b) {
        return;
    }
    char str[(sizeof(state.escapeseq) / sizeof(state.escapeseq[0])) + 1];
    str[(sizeof(state.escapeseq) / sizeof(state.escapeseq[0]))] = '\0';
    memcpy(str, &state.escapeseq[3], (sizeof(state.escapeseq) / sizeof(state.escapeseq[0])) - 3);

    int arg_count = countc(str, ';') + 1;

    char *arg_arr[arg_count];
    char *strptr = str;
    for (int i = 0; i < arg_count; i++) {
        arg_arr[i] = strptr;
        while (*strptr && *strptr != ';') {
            strptr++;
        }
        strptr++;
    }

    unsigned long values[arg_count];

    for (int i = 0; i < arg_count; i++) {
        values[i] = strtoul(arg_arr[i], nullptr, 10);
    }

    if (arg_count < 1) {
        return;
    }

    if (values[0] == 0) {
        color_background[0] = 0;
        color_background[1] = 0;
        color_background[2] = 0;
        color_foreground[0] = 255;
        color_foreground[1] = 255;
        color_foreground[2] = 255;
    }

    if (arg_count < 2) { // checks rely on this
        return;
    }

    if ((values[0] == 48 || values[0] == 38) && values[1] == 5 && arg_count >= 3) {
        uint8_t *colorptr;
        if (values[0] == 48) {
            colorptr = color_background;
        } else {
            colorptr = color_foreground;
        }
        values[2] %= 256;

        colorptr[2] = col256[values[2] - 16] & 0xFF;
        colorptr[1] = (col256[values[2] - 16] >> 8) & 0xFF;
        colorptr[0] = (col256[values[2] - 16] >> 16) & 0xFF;
    } else if ((values[0] == 48 || values[0] == 38) && values[1] == 2 && arg_count >= 5) {
        uint8_t *colorptr;
        if (values[0] == 48) {
            colorptr = color_background;
        } else {
            colorptr = color_foreground;
        }

        colorptr[0] = values[2];
        colorptr[1] = values[3];
        colorptr[2] = values[4];
    } else if (values[0] == 1 && arg_count >= 2) {
        // default
        if (values[1] == 39) {
            color_foreground[0] = 255;
            color_foreground[1] = 255;
            color_foreground[2] = 255;
            return;
        }
        if (values[1] == 49) {
            color_background[0] = 0;
            color_background[1] = 0;
            color_background[2] = 0;
            return;
        }
        if (values[1] == 0) {
            color_background[0] = 0;
            color_background[1] = 0;
            color_background[2] = 0;
            color_foreground[0] = 255;
            color_foreground[1] = 255;
            color_foreground[2] = 255;
            return;
        }

        uint8_t *colorptr = nullptr;
        if (values[1] >= 30 && values[1] <= 38) {
            colorptr = color_foreground;
            values[1] -= 30;
        } else if (values[1] >= 40 && values[1] <= 48) {
            colorptr = color_background;
            values[1] -= 40;
        }

        if (colorptr == nullptr) {
            return;
        }

        values[1] %= 16;

        colorptr[2] = col_ansi[values[1]] & 0xFF;
        colorptr[1] = (col_ansi[values[1]] >> 8) & 0xFF;
        colorptr[0] = (col_ansi[values[1]] >> 16) & 0xFF;
    }
}

void fb::fbconsole::putc(char c) {
    if (state.escaped) {
        if (state.escapeseq_index < (sizeof(state.escapeseq) / sizeof(state.escapeseq[0]))) {
            state.escapeseq[state.escapeseq_index++] = c;
        }
        if (c == 'm') {
            parse_esc();
            state.escaped = false;
            state.escapeseq_index = 0;
        }
    } else {
        if (c == '\x1b') {
            state.escaped = true;
            state.escapeseq[state.escapeseq_index++] = c;
            state.escapeseq_index++;
            return;
        }
        putc_screen(c);
    }
}

void fb::fbconsole::putc_screen(char c) {
    // special characters
    switch (c) {
    case '\n':
        linebreak();
        return;
    case '\t':
        for (int i = 0; i < 4; i++) {
            this->putc_screen(' ');
        }
        return;
    case '\r':
        return;
    case '\b':
        // erasing on the past line is hard because we do not have any text buffer...
        if (pos_x >= psfreader.get_width()) {
            pos_x -= psfreader.get_width();
        }
        return;
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
                _framebuffer->write_pixel(x - 1, y, color_foreground[0], color_foreground[1], color_foreground[2]);
            } else {
                _framebuffer->write_pixel(x - 1, y, color_background[0], color_background[1], color_background[2]);
            }
            count++;
        }
    }
    if ((pos_x + psfreader.get_width() + psfreader.get_width()) < _framebuffer->get_width()) {
        pos_x += psfreader.get_width();
    } else {
        linebreak();
    }
}
