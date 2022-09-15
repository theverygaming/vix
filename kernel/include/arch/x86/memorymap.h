#pragma once
#include <types.h>

#define MEMMAP_MAX_ENTRIES 40

namespace memorymap {
    void initMemoryMap(void *mapadr, int entrycount);
    typedef struct __attribute__((packed)) {
        uint64_t Base;
        uint64_t Length;
        uint32_t Type; // entry Type
        uint32_t reserved;
    } SMAP_entry;

    typedef struct __attribute__((packed)) {
        uint64_t start;
        uint64_t end;
        uint32_t type;
    } MemMapEntry;

    extern SMAP_entry map_entries[MEMMAP_MAX_ENTRIES];
    extern int map_entrycount;
}