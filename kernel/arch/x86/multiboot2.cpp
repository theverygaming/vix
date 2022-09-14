#include <arch/x86/multiboot2.h>
#include <panic.h>
#include <stdio.h>
#include <types.h>

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t size;
} multiboot2_tag_t;

static bool multiboot_find_tag(void *multiboot2_info_adr, uint32_t type, multiboot2_tag_t **tag_ptr) {
    // uint32_t multiboot_size = *(uint32_t *)multiboot2_info_adr;
    multiboot2_tag_t *tag = (multiboot2_tag_t *)((char *)multiboot2_info_adr + (sizeof(uint32_t) * 2));
    while (!(tag->type == 0 && tag->size == 8)) {
        // printf("found tag %u size: %u\n", tag->type, tag->size);
        if (tag->type == type) {
            *tag_ptr = tag;
            return true;
        }
        tag = (multiboot2_tag_t *)((char *)tag + ((tag->size + 7) & ~7)); // get next tag
    }
    return false;
}

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} multiboot2_memmap_tag_t;

void *multiboot2::findMemMap(void *multiboot2_info_adr, int *memmap_entrycount) {
    multiboot2_memmap_tag_t *tagptr;
    if (multiboot_find_tag(multiboot2_info_adr, 6, (multiboot2_tag_t **)&tagptr)) {
        // TODO: check if tag version is compatible
        *memmap_entrycount = (tagptr->size - 16) / tagptr->entry_size;
        return (void *)((char *)tagptr + 16);
    } else if (multiboot_find_tag(multiboot2_info_adr, 17, (multiboot2_tag_t **)&tagptr)) {
        KERNEL_PANIC("only found EFI memory map tag");
    } else {
        KERNEL_PANIC("literally memorymapless");
    }
    *memmap_entrycount = 0;
    return 0;
}
