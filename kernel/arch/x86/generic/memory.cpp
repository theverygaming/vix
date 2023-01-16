#include <arch/generic/archspecific.h>
#include <arch/generic/memory.h>
#include <arch/memorymap.h>
#include <arch/paging.h>

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
    if (n == 0) { // lets make zero the kernel memory entry
        entry->start_address = KERNEL_PHYS_ADDRESS;
        entry->size = KERNEL_FREE_AREA_BEGIN_OFFSET;
        entry->entry_type = memory_map_entry::entry_type::MEMORY_KERNEL;
        return true;
    }
    int memmap_index = n - 1;

    if (memmap_index >= memorymap::map_entrycount) {
        return false;
    }

    if ((memorymap::map_entries[memmap_index].Base >= 0xFFFFFFFF) && (sizeof(size_t) == 4)) { // in case of(future) 64-bit code
        if (memmap_index >= memorymap::map_entrycount) {
            return false;
        }
        return arch::generic::memory::get_memory_map(entry, n + 1);
    }

    entry->start_address = memorymap::map_entries[memmap_index].Base;
    entry->size = memorymap::map_entries[memmap_index].Length;

    if (memorymap::map_entries[memmap_index].Length > 0xFFFFFFFF) {
        entry->size = 0xFFFFFFFF;
    }

    if ((uint64_t)entry->start_address + (uint64_t)entry->size > 0xFFFFFFFF) {
        entry->size = 0xFFFFFFFF - (uint64_t)entry->start_address;
    }

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

void arch::generic::memory::vm_map(void *virt, void *phys, size_t pages, bool global, bool kernel) {
    paging::map_page(phys, virt, pages, false, global); // TODO: permissions
}

void arch::generic::memory::vm_unmap(void *virt) {
    paging::clearPageTables(virt, 1);
}

void *arch::generic::memory::vm_get_phys_address(void *virt) {
    return paging::get_physaddr_unaligned(virt);
}
