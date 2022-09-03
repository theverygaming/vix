#pragma once

namespace multiboot2 {
    /* returns pointer to memory map, takes pointer to multiboot2 struct and pointer to int for memory map entires */
    void *findMemMap(void *multiboot2_info_adr, int *memmap_entrycount);
}