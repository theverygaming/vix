#include "memorymap.h"
#include "stdio.h"
#include "stdlib.h"


namespace memorymap {
    SMAP_entry map_entries[E820_MAX_ENTRIES];
    int map_entrycount = -1;
}

void memorymap::initMemoryMap(void* countadr, void* mapadr) {
    uint16_t count = *((uint16_t*)countadr);
    //printf("e820 entry count: %d\n", count);
    SMAP_entry* entries = (SMAP_entry*)mapadr;
    MemMapEntry MemMap[count];
    if(count > E820_MAX_ENTRIES) {
        printf("%d e820 entries, E820_MAX_ENTRIES too low(%d), go change it bruh\n", count, E820_MAX_ENTRIES);
        asm("hlt");
    }
    map_entrycount = count;
    memcpy((char*)&map_entries, (char*)entries, count * sizeof(*map_entries));
    for(int i = 0; i < count; i++) {
        //printf("#%d -> base: %llu, length: %llu t: %u, acpi: %u\n", i, entries[i].Base, entries[i].Length, entries[i].Type, entries[i].ACPI);
        MemMap[i] = {entries[i].Base, entries[i].Base + entries[i].Length, entries[i].Type};
    }

    uint64_t totalMemory = 0;
    uint64_t totalUsableMemory = 0;
    for(int i = 0; i < count; i++) {
        //char arr[8][20] = { "", "usable", "system reserved", "ACPI reclaim", "ACPI NVS" , "Memory error", "disabled", "Persistent"};
        //printf("Memory Area: %llu -> %llu type: %u(%s)\n", MemMap[i].start, MemMap[i].end, MemMap[i].type, arr[MemMap[i].type]);
        if(MemMap[i].type == 1) {
            totalUsableMemory += MemMap[i].end - MemMap[i].start;
        }
        totalMemory += MemMap[i].end - MemMap[i].start;
    }
    printf("Total Memory: %uMB, usable memory: %uMB\n", (uint32_t)totalMemory / 1000000, (uint32_t)totalUsableMemory / 1000000);
}