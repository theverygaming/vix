#include <arch/generic/archspecific.h>
#include <arch/generic/memory.h>
#include <arch/memmap.h>

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
    if (n == 0) { // lets make zero the kernel memory entry
        entry->start_address = 0x80000;
        entry->size = 0xF0000;
        entry->entry_type = memory_map_entry::entry_type::MEMORY_KERNEL;
        return true;
    }
    int memmap_index = n - 1;

    if (memmap_index >= memory_map_entrycount) {
        return false;
    }

    switch (memory_map[memmap_index].type) {
    case memory_type::MEM_RAM:
        entry->entry_type = memory_map_entry::entry_type::MEMORY_RAM;
        break;
    case memory_type::MEM_DEVICE:
        entry->entry_type = memory_map_entry::entry_type::MEMORY_MIMO;
        break;
    default:
        entry->entry_type = memory_map_entry::entry_type::MEMORY_UNUSABLE;
        break;
    }
    entry->start_address = memory_map[memmap_index].start;
    entry->start_address = memory_map[memmap_index].size;
    return true;
}
