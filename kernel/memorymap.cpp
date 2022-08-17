#include "memorymap.h"
#include "stdio.h"
#include "stdlib.h"
#include <debug.h>

namespace memorymap {
    SMAP_entry map_entries[E820_MAX_ENTRIES];
    int map_entrycount = -1;
}

void memorymap::initMemoryMap(void *countadr, void *mapadr) {
    uint16_t count = *((uint16_t *)countadr);
    SMAP_entry *entries = (SMAP_entry *)mapadr;
    // MemMapEntry MemMap[count];
    MemMapEntry *MemMap = (MemMapEntry *)__builtin_alloca(count * sizeof(MemMapEntry));
    if (count > E820_MAX_ENTRIES) {
        printf("%d e820 entries, E820_MAX_ENTRIES too low(%d), go change it bruh\n", count, E820_MAX_ENTRIES);
        debug::debug_loop();
    }
    map_entrycount = count;
    memcpy((char *)&map_entries, (char *)entries, count * sizeof(*map_entries));
    char types[][20] = { "", "usable", "system reserved", "ACPI reclaim", "ACPI NVS" , "Memory error", "disabled", "Persistent"};
    for (int i = 0; i < count; i++) {
        DEBUG_PRINTF("#%d -> base: %llu, length: %llu type: %u(%s), acpi: %u\n", i, entries[i].Base, entries[i].Length, entries[i].Type, types[entries[i].Type], entries[i].ACPI);
        MemMap[i] = {entries[i].Base, entries[i].Base + entries[i].Length, entries[i].Type};
    }

    uint64_t totalMemory = 0;
    uint64_t totalUsableMemory = 0;
    for (int i = 0; i < count; i++) {
        if (MemMap[i].type == 1) {
            totalUsableMemory += MemMap[i].end - MemMap[i].start;
        }
        totalMemory += MemMap[i].end - MemMap[i].start;
    }
    printf("Total Memory: %uMB, usable memory: %uMB\n", (uint32_t)totalMemory / 1000000, (uint32_t)totalUsableMemory / 1000000);
    if(totalUsableMemory > 0xFFFFFFFF) {
        printf("^ about that... you have more memory than this, but i have no 64-bit divide function sooo we can't display it here\n");
    }
}