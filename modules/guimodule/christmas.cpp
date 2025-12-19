#include <algorithm>
#include <stdint.h>
#include <vix/arch/generic/devices.h>
#include <vix/framebuffer.h>
#include <vix/sched.h>
#include <vix/time.h>
#include <vix/types.h>

extern fb::fb framebuffer;

static unsigned int rand_determ(unsigned int *seed) {
    (*seed) = (42069 * (*seed) + 1) % (-1);
    return (*seed);
}

static bool
in_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y) {
    return (x1 - x2) * (y - y1) - (y1 - y2) * (x - x1) > 0 &&
           (x2 - x3) * (y - y2) - (y2 - y3) * (x - x2) > 0 &&
           (x3 - x1) * (y - y3) - (y3 - y1) * (x - x3) > 0;
}

// silly triangle drawing algo, it's slow as fuck and not really that accurate as-is _but_ it'll do here! - We only draw it once anyway
// https://web.archive.org/web/20050408192410/http://sw-shader.sourceforge.net/rasterizer.html
static void draw_triangle(
    size_t x1,
    size_t y1,
    size_t x2,
    size_t y2,
    size_t x3,
    size_t y3,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    size_t xmax = std::max(std::max(x1, x2), x3);
    size_t xmin = std::min(std::min(x1, x2), x3);
    size_t ymax = std::max(std::max(y1, y2), y3);
    size_t ymin = std::min(std::min(y1, y2), y3);

    for (size_t y = ymin; y <= ymax; y++) {
        for (size_t x = xmin; x <= xmax; x++) {
            if (in_triangle(x1, y1, x2, y2, x3, y3, x, y)) {
                framebuffer.write_pixel(x, y, r, g, b);
            }
        }
    }
}

static void draw_rectangle(
    size_t tl_x,
    size_t tl_y,
    size_t width,
    size_t height,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    for (size_t y = tl_y; y <= tl_y + height; y++) {
        for (size_t x = tl_x; x <= tl_x + width; x++) {
            framebuffer.write_pixel(x, y, r, g, b);
        }
    }
}

static void
draw_circle(int mid_x, int mid_y, int radius, uint8_t r, uint8_t g, uint8_t b) {
    int r2 = radius * radius;
    for (int y = -radius; y <= radius; y++) {
        int y2 = y * y;
        for (int x = -radius; x <= radius; x++) {
            int x2 = x * x;
            if (x2 + y2 <= r2) {
                framebuffer.write_pixel(mid_x + x, mid_y + y, r, g, b);
            }
        }
    }
}

static void tree_generator() {
    size_t top_x = 700;
    size_t top_y = 50;
    size_t width = 100;
    size_t height = 150;
    // the points need to be in counter-clockwise order!
    // top
    size_t t_x1 = top_x;
    size_t t_y1 = top_y;
    // left
    size_t t_x2 = top_x - (width / 2);
    size_t t_y2 = top_y + height;
    // right
    size_t t_x3 = top_x + (width / 2);
    size_t t_y3 = top_y + height;

    uint8_t tree_r = 7;
    uint8_t tree_g = 86;
    uint8_t tree_b = 0;

    size_t width_base = 20;
    size_t height_base = 40;

    // blinky lights
    size_t light_radius_max = 2;
    uint8_t blend_ctr = 0;
    int8_t blend_add = 1;
    while (true) {
        // tree
        draw_triangle(
            t_x1, t_y1, t_x2, t_y2, t_x3, t_y3, tree_r, tree_g, tree_b
        );

        // base
        draw_rectangle(
            top_x - (width_base / 2), t_y3, width_base, height_base, 86, 50, 50
        );

        unsigned int seed = 0;
        for (int i = 0; i < 50; i++) {
            size_t x;
            size_t y;
            while (true) {
                x = t_x2 + (rand_determ(&seed) % width);
                y = top_y + (rand_determ(&seed) % height);
                if (in_triangle(
                        t_x1,
                        t_y1,
                        t_x2,
                        t_y2,
                        t_x3,
                        t_y3,
                        x - light_radius_max,
                        y - light_radius_max
                    ) &&
                    in_triangle(
                        t_x1,
                        t_y1,
                        t_x2,
                        t_y2,
                        t_x3,
                        t_y3,
                        x + light_radius_max,
                        y + light_radius_max
                    ) &&
                    in_triangle(
                        t_x1,
                        t_y1,
                        t_x2,
                        t_y2,
                        t_x3,
                        t_y3,
                        x - light_radius_max,
                        y + light_radius_max
                    )) {
                    break;
                }
            }
            // TODO: it's hard to keep the breathing effect, but that would be cool
            uint8_t local_blend = blend_ctr + rand_determ(&seed);

            uint8_t r = 240;
            uint8_t g = 219;
            uint8_t b = 77;
            if ((rand_determ(&seed) & 0b11) == 2) {
                r = 170;
                g = 0;
                b = 0;
            }

            draw_circle(x, y, local_blend / (255 / light_radius_max), r, g, b);
        }
        blend_ctr += blend_add;
        if (blend_ctr == 255) {
            blend_add = -1;
        } else if (blend_ctr == 0) {
            blend_add = 1;
        }
        // wait 100ms
        uint64_t ns = time::ns_since_bootup;
        while ((time::ns_since_bootup - ns) < 1000000 * 100) {
            sched::yield();
        }
    }
}

void christmas_init() {
    uint16_t year;
    uint8_t month, day, hour, minute, second;
    arch::generic::devices::get_current_rtc_time(
        &year, &month, &day, &hour, &minute, &second
    );
    // FIXME: start_thread should prolly return a PID!
    if (month == 12 && day >= 15 && day <= 29) {
        sched::start_thread(tree_generator);
    }
}

void christmas_deinit() {
    // FIXME: actually kill it
}
