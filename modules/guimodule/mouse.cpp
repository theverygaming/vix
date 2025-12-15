#include "mouse.h"
#include <algorithm>
#include <vix/drivers/ms_mouse.h>
#include <vix/framebuffer.h>

extern fb::fb framebuffer;

static inline uint8_t bitget(uint32_t num, uint8_t bit) {
    return (num & (0x1 << bit)) >> bit;
}

constexpr size_t mouse_w = 8;
constexpr size_t mouse_h = 8;

static const uint8_t cursor[8] = {
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
static bool restorebg = false;
static uint8_t mouse_bg[mouse_w * mouse_h * 3];

static void mouse_event(void *, struct drivers::ms_mouse::mouse_packet packet) {
    if (restorebg && mouse_x + mouse_w < framebuffer.get_width() && mouse_y + mouse_h < framebuffer.get_height()) {
        for (int y = 0; y < mouse_h; y++) {
            for (int x = 0; x < mouse_w; x++) {
                int index_o = (y * mouse_w * 3) + (x * 3);
                framebuffer.write_pixel(mouse_x + x, mouse_y + y, mouse_bg[index_o], mouse_bg[index_o + 1], mouse_bg[index_o + 2]);
            }
        }
    }

    mouse_x += packet.x_movement;
    mouse_y -= packet.y_movement; // inverted for Y

    mouse_x = std::clamp(mouse_x, (ssize_t)0, (ssize_t)framebuffer.get_width());
    mouse_y = std::clamp(mouse_y, (ssize_t)0, (ssize_t)framebuffer.get_height());

    if (packet.lmb) {
        for (size_t y = 0; y < mouse_h; y++) {
            for (size_t x = 0; x < mouse_w; x++) {
                size_t p_x = (mouse_x+x);
                size_t p_y = (mouse_y+y);
                if (p_x < framebuffer.get_width() && p_y < framebuffer.get_height()) {
                    framebuffer.write_pixel(p_x, p_y, p_x+p_y, p_x-p_y, p_x*p_y);
                }
            }
        }
    }

    if (mouse_x + mouse_w < framebuffer.get_width() && mouse_y + mouse_h < framebuffer.get_height()) {
        for (size_t y = 0; y < mouse_h; y++) {
            for (size_t x = 0; x < mouse_w; x++) {
                size_t index_o = (y * mouse_w * 3) + (x * 3);
                framebuffer.read_pixel(mouse_x + x, mouse_y + y, &mouse_bg[index_o], &mouse_bg[index_o + 1], &mouse_bg[index_o + 2]);
                if (bitget(cursor[y], x)) {
                    framebuffer.write_pixel(mouse_x + x, mouse_y + y, 255, 255, 255);
                }
            }
        }
        restorebg = true;
    }
}

void mouse_init() {
    drivers::ms_mouse::register_event_listener(mouse_event, nullptr);
}

void mouse_deinit() {
    drivers::ms_mouse::deregister_event_listener(mouse_event, nullptr);
}
