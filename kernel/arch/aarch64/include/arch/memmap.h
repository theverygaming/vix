#pragma once

#include <types.h>

enum class memory_type { MEM_ROM, MEM_RAM, MEM_RAM_UNUSABLE, MEM_DEVICE };

typedef struct {
    uintptr_t start;
    uintptr_t size;
    memory_type type;
} memmap_entry_t;

extern int memory_map_entrycount;
extern memmap_entry_t memory_map[];
