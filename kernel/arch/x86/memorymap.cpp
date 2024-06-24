#include <arch/memorymap.h>
#include <debug.h>
#include <kprintf.h>
#include <mm/memmap.h>
#include <panic.h>
#include <string.h>

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

static struct mm::mem_map_entry converted_entries[MEMMAP_MAX_ENTRIES];

static mm::mem_map_entry::type_t convert_from_e820(uint32_t e820_type) {
    switch (e820_type) {
    case 1:
        return mm::mem_map_entry::type_t::RAM;
    case 2:
        return mm::mem_map_entry::type_t::RESERVED;
    case 3:
        return mm::mem_map_entry::type_t::ACPI_RECLAIM;
    case 4:
        return mm::mem_map_entry::type_t::ACPI_NVS;
    case 5:
        return mm::mem_map_entry::type_t::UNUSABLE;
    case 6:
        return mm::mem_map_entry::type_t::DISABLED;
    case 7:
        return mm::mem_map_entry::type_t::PERSISTENT;
    default:
        return mm::mem_map_entry::type_t::UNKNOWN;
    }
}

void memorymap::initMemoryMap(void *mapadr, int entrycount) {
    memset(converted_entries, 0, MEMMAP_MAX_ENTRIES * sizeof(struct mm::mem_map_entry));
    if (entrycount > MEMMAP_MAX_ENTRIES) {
        kprintf(KP_EMERG, "memmap: got %d memory map entries, MEMMAP_MAX_ENTRIES too low(%d)\n", entrycount, MEMMAP_MAX_ENTRIES);
        KERNEL_PANIC("memory map issue");
    }
    SMAP_entry *entries = (SMAP_entry *)mapadr;
    memcpy(&map_entries, entries, entrycount * sizeof(map_entries[0]));
    for (int i = 0; i < entrycount; i++) {
        DEBUG_PRINTF("#%d -> base: %u, length: %u type: %u\n",
                     i,
                     (uint32_t)(map_entries[i].Base & 0xFFFFFFFF),
                     (uint32_t)(map_entries[i].Length & 0xFFFFFFFF),
                     map_entries[i].Type);
        converted_entries[i].base = map_entries[i].Base;
        converted_entries[i].size = map_entries[i].Length;
        converted_entries[i].type = convert_from_e820(map_entries[i].Type);
    }

    kprintf(KP_INFO, "memorymap: initialized\n");
    mm::set_mem_map(converted_entries, MEMMAP_MAX_ENTRIES);
}
