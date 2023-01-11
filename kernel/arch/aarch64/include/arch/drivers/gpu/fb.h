#pragma once
#include <framebuffer.h>
#include <types.h>

namespace drivers::gpu {
    bool setup_fb(size_t width, size_t height, size_t bpp, struct fb::fbinfo *info);
}
