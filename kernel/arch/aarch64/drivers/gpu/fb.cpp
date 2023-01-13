/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <arch/drivers/gpu/fb.h>
#include <arch/drivers/gpu/mbox.h>
#include <mm/kmalloc.h>
#include <stdio.h>
#include <types.h>

// https://github.com/bztsrc/raspi3-tutorial/blob/master/09_framebuffer/lfb.c
bool drivers::gpu::setup_fb(size_t width, size_t height, size_t bpp, struct fb::fbinfo *info) {
    // TODO: aligned free
    volatile uint32_t *buf = (volatile uint32_t *)mm::kmalloc_aligned(36 * 4, 16);
    buf[0] = 35 * 4;
    buf[1] = 0;

    buf[2] = 0x48003; // set phy wh
    buf[3] = 8;
    buf[4] = 8;
    buf[5] = width;  // FrameBufferInfo.width
    buf[6] = height; // FrameBufferInfo.height

    buf[7] = 0x48004; // set virt wh
    buf[8] = 8;
    buf[9] = 8;
    buf[10] = width;  // FrameBufferInfo.virtual_width
    buf[11] = height; // FrameBufferInfo.virtual_height

    buf[12] = 0x48009; // set virt offset
    buf[13] = 8;
    buf[14] = 8;
    buf[15] = 0; // FrameBufferInfo.x_offset
    buf[16] = 0; // FrameBufferInfo.y.offset

    buf[17] = 0x48005; // set depth
    buf[18] = 4;
    buf[19] = 4;
    buf[20] = bpp; // FrameBufferInfo.depth

    buf[21] = 0x48006; // set pixel order
    buf[22] = 4;
    buf[23] = 4;
    buf[24] = 1; // RGB, not BGR preferably

    buf[25] = 0x40001; // get framebuffer, gets alignment on request
    buf[26] = 8;
    buf[27] = 8;
    buf[28] = 4096; // FrameBufferInfo.pointer
    buf[29] = 0;    // FrameBufferInfo.size

    buf[30] = 0x40008; // get pitch
    buf[31] = 4;
    buf[32] = 4;
    buf[33] = 0; // FrameBufferInfo.pitch

    buf[34] = 0;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (mbox_call(MBOX_CH_PROPERTY, buf) && buf[28] != 0) {
        buf[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        printf("buf[28] = 0x%p\n", (uintptr_t)buf[28]);
        info->address = (void *)buf[28];
        info->width = buf[5];
        info->height = buf[6];
        info->pitch = buf[33];
        info->bpp = buf[20];
        info->rgb = buf[24] == 1;
        return true;
    } else {
        printf("buf[28] = 0x%p\n", (uintptr_t)buf[28]);
        puts("failure\n");
        return false;
    }

    // mm::kfree_aligned(buf);
}
