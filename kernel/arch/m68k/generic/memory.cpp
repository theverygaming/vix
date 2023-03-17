#include <arch/generic/archspecific.h>
#include <arch/generic/memory.h>

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
    return false;
    /*
    if (n == 0) { // lets make zero the kernel memory entry
        entry->start_address = 0x80000;
        entry->size = 0xF0000;
        entry->entry_type = memory_map_entry::entry_type::MEMORY_KERNEL;
        return true;
    }
    int memmap_index = n - 1;

    if (memmap_index == 0) {
        entry->start_address = ARCH_KERNEL_HEAP_START;
        entry->size = ARCH_KERNEL_HEAP_END - ARCH_KERNEL_HEAP_START;
        entry->entry_type = memory_map_entry::entry_type::MEMORY_RAM;
        return true;
    }

    if (memmap_index >= memory_map_entrycount) {
        return false;
    }

    return false;

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
    entry->size = memory_map[memmap_index].size;
    return true;
    */
}
