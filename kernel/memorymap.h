#pragma once
#include "types.h"

namespace memorymap {
    void initMemoryMap(void* countadr, void* mapadr);
    typedef struct SMAP_entry {
        uint64_t Base;
        uint64_t Length;
	    uint32_t Type; // entry Type
	    uint32_t ACPI; // extended
    }__attribute__((packed));
    typedef struct MemMapEntryx {
        uint64_t start;
        uint64_t end;
        uint32_t type;
    } MemMapEntry;
}