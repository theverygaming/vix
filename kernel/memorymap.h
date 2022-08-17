#pragma once
#include "types.h"

#define E820_MAX_ENTRIES 20

namespace memorymap {
    void initMemoryMap(void *countadr, void *mapadr);
    typedef struct __attribute__((packed)) {
        uint64_t Base;
        uint64_t Length;
        uint32_t Type; // entry Type
        uint32_t ACPI; // extended
    } SMAP_entry;

    typedef struct {
        uint64_t start;
        uint64_t end;
        uint32_t type;
    } MemMapEntry;

    extern SMAP_entry map_entries[E820_MAX_ENTRIES];
    extern int map_entrycount;
}