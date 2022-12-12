#pragma once
#include <framebuffer.h>
#include <types.h>

namespace multiboot2 {
    /* returns pointer to memory map, takes pointer to multiboot2 struct and pointer to int for memory map entires */
    void *findMemMap(void *multiboot2_info_adr, int *memmap_entrycount);

    struct fb::fbinfo findFrameBuffer(void *multiboot2_info_adr);
}
