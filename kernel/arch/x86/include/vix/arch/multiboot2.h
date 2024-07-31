#pragma once
#include <vix/framebuffer.h>
#include <vix/types.h>

namespace multiboot2 {
    /* returns pointer to memory map, takes pointer to multiboot2 struct and pointer to int for memory map entries */
    void *findMemMap(void *multiboot2_info_adr, int *memmap_entrycount);

    struct fb::fbinfo findFrameBuffer(const void *multiboot2_info_adr);
    bool find_initramfs(const void *multiboot2_info_adr, void **start, size_t *size);
}
