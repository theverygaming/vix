#pragma once
#include "types.h"
#include "memorymap.h"

namespace memalloc::page {
    void* malloc(uint32_t blocks);
    void free(void* adr);
    void init(memorymap::SMAP_entry* e620_map, int e620_len);
}