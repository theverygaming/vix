#include <arch/generic/archspecific.h>
#include <arch/generic/memory.h>
#include <arch/multiboot2.h>
#include <arch/paging.h>
#include <debug.h>
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <panic.h>
#include <stdio.h>
#include <types.h>

struct __attribute__((packed)) multiboot2_tag {
    uint32_t type;
    uint32_t size;
};

static bool multiboot_find_tag(const void *multiboot2_info_adr, uint32_t type, struct multiboot2_tag **tag_ptr) {
    // uint32_t multiboot_size = *(uint32_t *)multiboot2_info_adr;
    struct multiboot2_tag *tag = (struct multiboot2_tag *)((const char *)multiboot2_info_adr + (sizeof(uint32_t) * 2));
    while (!(tag->type == 0 && tag->size == 8)) {
        // printf("found tag %u size: %u\n", tag->type, tag->size);
        if (tag->type == type) {
            *tag_ptr = tag;
            return true;
        }
        tag = (struct multiboot2_tag *)((char *)tag + ((tag->size + 7) & ~7)); // get next tag
    }
    return false;
}

typedef struct __attribute__((packed)) {
    uint32_t type; // 6
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} multiboot2_memmap_tag_t;

void *multiboot2::findMemMap(void *multiboot2_info_adr, int *memmap_entrycount) {
    multiboot2_memmap_tag_t *tagptr;
    if (multiboot_find_tag(multiboot2_info_adr, 6, (struct multiboot2_tag **)&tagptr)) {
        // TODO: check if tag version is compatible
        *memmap_entrycount = (tagptr->size - sizeof(multiboot2_memmap_tag_t)) / tagptr->entry_size;
        if (tagptr->entry_size != 24) {
            KERNEL_PANIC("incompatible mb2 memory map");
        }
        return (void *)((char *)tagptr + sizeof(multiboot2_memmap_tag_t));
    } else if (multiboot_find_tag(multiboot2_info_adr, 17, (struct multiboot2_tag **)&tagptr)) {
        KERNEL_PANIC("only found EFI memory map tag");
    } else {
        KERNEL_PANIC("literally memorymapless");
    }
    *memmap_entrycount = 0;
    return 0;
}

struct __attribute__((packed)) multiboot2_framebuffer_info_tag {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved;
};

struct fb::fbinfo multiboot2::findFrameBuffer(const void *multiboot2_info_adr) {
    struct multiboot2_framebuffer_info_tag *tag;
    if (multiboot_find_tag(multiboot2_info_adr, 8, (struct multiboot2_tag **)&tag)) {
        assertm(tag->framebuffer_type != 2, "wrong framebuffer type");
        size_t fb_bytes = tag->framebuffer_pitch * tag->framebuffer_height;
        if (fb_bytes % ARCH_PAGE_SIZE != 0) {
            fb_bytes += fb_bytes % ARCH_PAGE_SIZE;
        }

        if (tag->framebuffer_addr > 0xFFFFFFFF) {
            KERNEL_PANIC("framebuffer is in 64-bit address space");
        }

        // rather hacky because we have no VMM
        void *fb_virt_adr = (void *)(KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET);
        //mm::pmm::force_alloc_contiguous((void *)((uintptr_t)tag->framebuffer_addr), fb_bytes / ARCH_PAGE_SIZE);
        paging::map_page(
            (void *)((uintptr_t)tag->framebuffer_addr), (void *)(KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET), fb_bytes / ARCH_PAGE_SIZE);
        return {
            .address = fb_virt_adr,
            .width = tag->framebuffer_width,
            .height = tag->framebuffer_height,
            .pitch = tag->framebuffer_pitch,
            .bpp = tag->framebuffer_bpp,
            .rgb = false,
        };
    }
    KERNEL_PANIC("couldn't initialize framebuffer");
    return {};
}

struct __attribute__((packed)) boot_module_tag {
    uint32_t type; // 3
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
};

bool multiboot2::find_initramfs(const void *multiboot2_info_adr, void **start, size_t *size) {
    struct boot_module_tag *tag;
    if (multiboot_find_tag(multiboot2_info_adr, 3, (struct multiboot2_tag **)&tag)) {
        *start = (void *)tag->mod_start;
        *size = tag->mod_end - tag->mod_start;
        return true;
    }
    return false;
}
