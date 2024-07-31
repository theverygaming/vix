#pragma once
#include <vix/framebuffer.h>
#include <vix/types.h>

namespace drivers::gpu {
    bool setup_fb(size_t width, size_t height, size_t bpp, struct fb::fbinfo *info);
}
