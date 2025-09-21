#pragma once
#include <vix/types.h>
#include <vix/mm/memmap.h>

namespace memorymap {
    void initMemoryMap(void *mapadr, size_t entrycount, struct mm::mem_map_entry (*extra_entries)(size_t n), size_t extra_entry_count);
}
