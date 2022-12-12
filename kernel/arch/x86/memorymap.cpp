#include <arch/memorymap.h>
#include <debug.h>
#include <log.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint64_t start;
    uint64_t end;
    uint32_t type;
} MemMapEntry;

namespace memorymap {
    SMAP_entry map_entries[MEMMAP_MAX_ENTRIES];
    int map_entrycount = MEMMAP_MAX_ENTRIES;
    size_t total_ram;
}

void memorymap::initMemoryMap(void *mapadr, int entrycount) {
    if (entrycount > MEMMAP_MAX_ENTRIES) {
        printf("%d memory map entries, MEMMAP_MAX_ENTRIES too low(%d), go change it bruh\n", entrycount, MEMMAP_MAX_ENTRIES);
        KERNEL_PANIC("memory map issue");
    }
    SMAP_entry *entries = (SMAP_entry *)mapadr;
    memcpy(&map_entries, entries, entrycount * sizeof(*map_entries));
    MemMapEntry processed[entrycount];
    char types[][20] = {"", "usable", "system reserved", "ACPI reclaim", "ACPI NVS", "Memory error", "disabled", "Persistent"};
    for (int i = 0; i < entrycount; i++) {
        DEBUG_PRINTF("#%d -> base: %llu, length: %llu type: %u(%s)\n", i, entries[i].Base, entries[i].Length, entries[i].Type, types[entries[i].Type]);
        processed[i] = {entries[i].Base, entries[i].Base + entries[i].Length, entries[i].Type};
    }

    uint64_t totalMemory = 0;
    uint64_t totalUsableMemory = 0;
    for (int i = 0; i < entrycount; i++) {
        if (processed[i].type == 1) {
            totalUsableMemory += processed[i].end - processed[i].start;
        }
        totalMemory += processed[i].end - processed[i].start;
    }
    total_ram = totalUsableMemory;
    DEBUG_PRINTF("Total Memory: %uMB, usable memory: %uMB\n", (uint32_t)totalMemory / 1000000, (uint32_t)totalUsableMemory / 1000000);
    if (totalUsableMemory > 0xFFFFFFFF) {
        DEBUG_PRINTF("^ about that... you have more memory than this, but i have no 64-bit divide function sooo we can't display it here\n");
        if (sizeof(size_t) == 4) {
            total_ram = 0xFFFFFFFF;
        }
    }
    log::log_service("memorymap", "initialized");
}
