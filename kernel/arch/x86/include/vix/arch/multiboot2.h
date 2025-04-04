#pragma once
#include <vix/framebuffer.h>
#include <vix/types.h>

namespace multiboot2 {
    /* get size of multiboot2 info structure */
    size_t get_tags_size(void *multiboot2_info_adr);

    /* returns pointer to memory map, takes pointer to multiboot2 struct and pointer to int for memory map entries */
    void *findMemMap(void *multiboot2_info_adr, int *memmap_entrycount);

    bool find_initramfs(const void *multiboot2_info_adr, void **start, size_t *size);

    /* NOTE: these functions can only be called after VMM & PMM are initialized */
    struct fb::fbinfo findFrameBuffer(const void *multiboot2_info_adr);
}
