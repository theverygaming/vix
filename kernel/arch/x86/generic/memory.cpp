#include <arch/generic/archspecific.h>
#include <arch/generic/memory.h>
#include <arch/memorymap.h>
#include <arch/paging.h>
#include <config.h>

#ifdef CONFIG_ENABLE_KERNEL_64
#include <arch/limine.h>
static volatile struct limine_memmap_request memmap_request = {.id = LIMINE_MEMMAP_REQUEST, .revision = 0};
#endif

bool arch::generic::memory::get_memory_map(struct memory_map_entry *entry, int n) {
#ifdef CONFIG_ENABLE_KERNEL_32
    int memmap_index = n - 1;
    if (n == 0) {
        entry->start_address = KERNEL_PHYS_ADDRESS;
        entry->size = KERNEL_FREE_AREA_BEGIN_OFFSET;
        entry->entry_type = memory_map_entry::entry_type::MEMORY_KERNEL;
        return true;
    }

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
#else
    if (memmap_request.response == NULL || n >= memmap_request.response->entry_count) {
        return false;
    }

    struct limine_memmap_entry *limine_entry = memmap_request.response->entries[n];

    entry->start_address = limine_entry->base;
    entry->size = limine_entry->length;
    entry->entry_type = memory_map_entry::entry_type::MEMORY_RAM;

    if (limine_entry->type != LIMINE_MEMMAP_USABLE) {
        entry->entry_type = memory_map_entry::entry_type::MEMORY_UNUSABLE;
    }

#endif
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
