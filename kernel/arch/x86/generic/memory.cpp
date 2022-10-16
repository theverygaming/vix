#include <arch/x86/generic/archspecific.h>
#include <arch/x86/generic/memory.h>
#include <arch/x86/memorymap.h>

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
    if (n == 0) { // lets make zero the kernel memory entry
        entry->start_address = KERNEL_PHYS_ADDRESS;
        entry->size =  KERNEL_MEMORY_END_OFFSET;
        entry->entry_type = memory_map_entry::entry_type::MEMORY_KERNEL;
        return true;
    }
    int memmap_index = n - 1;

    if (memmap_index >= memorymap::map_entrycount) {
        return false;
    }

    if ((memorymap::map_entries[memmap_index].Base >= 0xFFFFFFFF) && (sizeof(size_t) == 4)) { // in case of(future) 64-bit code
        memmap_index++;
        if (memmap_index >= memorymap::map_entrycount) {
            return false;
        }
    }

    entry->start_address = memorymap::map_entries[memmap_index].Base;
    entry->size = memorymap::map_entries[memmap_index].Length;
    switch (memorymap::map_entries[memmap_index].Type) {
    case 1:
        entry->entry_type = memory_map_entry::entry_type::MEMORY_RAM;
        break;
    default:
        entry->entry_type = memory_map_entry::entry_type::MEMORY_UNUSABLE;
        break;
    }
    return true;
}
