#include "memorymap.h"

#include "stdio.h"

void memorymap::initMemoryMap(void* countadr, void* mapadr) {
    uint16_t count = *((uint16_t*)countadr);
    printf("e820 entry count: %d\n", count);
    SMAP_entry* entries = (SMAP_entry*)mapadr;
    MemMapEntry MemMap[count];
    for(int i = 0; i < count; i++) {
        printf("#%d -> base: %llu, length: %llu t: %u, acpi: %u\n", i, entries[i].Base, entries[i].Length, entries[i].Type, entries[i].ACPI);
        MemMap[i] = {entries[i].Base, entries[i].Base + entries[i].Length, entries[i].Type};
    }

    uint64_t totalMemory = 0;
    uint64_t totalUsableMemory = 0;
    for(int i = 0; i < count; i++) {
        char arr[8][20] = { "", "usable", "system reserved", "ACPI reclaim", "ACPI NVS" , "Memory error", "disabled", "Persistent"};
        printf("Memory Area: %llu -> %llu type: %u(", MemMap[i].start, MemMap[i].end, MemMap[i].type);
        printf(arr[MemMap[i].type]);
        printf(")\n");
        if(MemMap[i].type == 1) {
            totalUsableMemory += MemMap[i].end - MemMap[i].start;
        }
        totalMemory += MemMap[i].end - MemMap[i].start;
    }
    printf("Total Memory: %llu bytes(%luMB), usable memory: %llu bytes(%luMB)\n", totalMemory, (uint32_t)totalMemory / 1000000, totalUsableMemory, (uint32_t)totalUsableMemory / 1000000);
}