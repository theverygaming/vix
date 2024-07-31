#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/memmap.h>
#include <vix/mm/pmm.h>
#include <vix/mm/vmm.h>
#include <vix/panic.h>
#include <vix/status.h>
#include <string.h>
#include <vix/types.h>

// TODO: there are lots of places to optimize in this class!!!
class bitmap {
public:
    bitmap() {}

    bitmap(void *ptr, size_t bytes) {
        _bitmap = (uint8_t *)ptr;
        _bitmap_size = bytes;
    }

    void clearAll() {
        memset(_bitmap, 0, _bitmap_size);
    }

    void setAll() {
        memset(_bitmap, 0xFF, _bitmap_size);
    }

    void setBit(size_t n, bool value) {
        size_t index = n / 8;
        size_t offset = n % 8;
        if (index >= _bitmap_size) {
            return;
        }
        bitset(&_bitmap[index], offset, value);
    }

    bool getBit(size_t n) {
        size_t index = n / 8;
        size_t offset = n % 8;
        if (index >= _bitmap_size) {
            return false;
        }
        return bitget(_bitmap[index], offset) != 0;
    }

    void setRange(size_t start, size_t n) {
        for (size_t i = start; i < (start + n); i++) {
            setBit(i, true);
        }
    }

    void unsetRange(size_t start, size_t n) {
        for (size_t i = start; i < (start + n); i++) {
            setBit(i, false);
        }
    }

    size_t countRange(size_t start, size_t n, bool set) {
        size_t count = 0;
        for (size_t i = start; i < (start + n); i++) {
            bool val = getBit(i);
            if (set ? val : !val) {
                count++;
            }
        }
        return count;
    }

    bool findRange(size_t start, size_t end, size_t n, bool set, size_t *found_start) {
        // impossible in the first place. abort
        if (n > (end - start)) {
            return false;
        }
        for (size_t i = start; i < end; i++) {
            // are we at the point where it would be impossbile to find more?
            if ((end - i) < n) {
                break;
            }
            size_t count = countRange(i, n, set);
            if (count == n) {
                *found_start = i;
                return true;
            }
        }
        return false;
    }

private:
    uint8_t *_bitmap;
    size_t _bitmap_size;

    /* helper functions */
    void bitset(uint8_t *byte, uint8_t bitnum, uint8_t value) {
        *byte ^= (-value ^ *byte) & (1 << bitnum);
    }

    uint8_t bitget(uint8_t byte, uint8_t bitnum) {
        return (byte >> bitnum) & 1;
    }
};

// PMM layout:
// pm_n_areas * area_info structs
// bitmap (managed via generic bitmap manager class)

struct __attribute__((packed)) area_info {
    uintptr_t start_addr;
    uintptr_t n_pages;
};

static uintptr_t get_memmap_required_space(uintptr_t *ptr_n_areas = nullptr, uintptr_t *ptr_n_pages = nullptr) {
    uintptr_t n_areas = 0;
    uintptr_t n_pages = 0;

    size_t counter = 0;
    const struct mm::mem_map_entry *entry = mm::get_mem_map(counter);
    while (entry != nullptr) {
        if (mm::memmap_is_usable(entry->type) && entry->size > ARCH_PAGE_SIZE) {
            uintptr_t start = ALIGN_UP(entry->base, ARCH_PAGE_SIZE);
            uintptr_t end = ALIGN_DOWN(entry->base + entry->size, ARCH_PAGE_SIZE);
            n_pages += (end - start) / ARCH_PAGE_SIZE;
            n_areas++;
        }
        counter++;
        entry = mm::get_mem_map(counter);
    }
    if (ptr_n_areas != nullptr) {
        *ptr_n_areas = n_areas;
    }
    if (ptr_n_pages != nullptr) {
        *ptr_n_pages = n_pages;
    }
    uintptr_t required_bytes = (ALIGN_UP(n_pages, 8) / 8) + (n_areas * sizeof(struct area_info));

    kprintf(KP_INFO, "pmm: %u pages to manage\n", n_pages);
    kprintf(KP_INFO, "pmm: need to reserve %u bytes for bitmap & interal structures\n", required_bytes);
    return required_bytes;
}

static const mm::mem_map_entry *get_suitable_memmap_entry(uintptr_t required_bytes) {
    size_t counter = 0;
    const struct mm::mem_map_entry *entry = mm::get_mem_map(counter);
    while (entry != nullptr) {
        if (mm::memmap_is_usable(entry->type) && entry->size > ARCH_PAGE_SIZE) {
            uintptr_t start = ALIGN_UP(entry->base, ARCH_PAGE_SIZE);
            uintptr_t end = ALIGN_DOWN(entry->base + entry->size, ARCH_PAGE_SIZE);
            if ((end - start) >= required_bytes) {
                kprintf(KP_INFO, "pmm: found suitable entry for bitmap -> base: 0x%p size: %u\n", (uintptr_t)entry->base, (uintptr_t)entry->size);
                break;
            }
        }
        counter++;
        entry = mm::get_mem_map(counter);
    }
    if (entry == nullptr) {
        KERNEL_PANIC("could not find suitable entry for PMM bitmap");
    }
    return entry;
}

static uintptr_t pm_n_total_pages;
static struct area_info *pm_areas;
static uintptr_t pm_n_areas;
static bitmap pm_bitmap;
static uintptr_t pm_phys_addr;

static void populate_pmm_info() {
    uintptr_t required_bytes = ALIGN_UP(get_memmap_required_space(&pm_n_areas, &pm_n_total_pages), ARCH_PAGE_SIZE);
    const mm::mem_map_entry *entry = get_suitable_memmap_entry(required_bytes);
    uintptr_t entry_phys_start = ALIGN_UP(entry->base, ARCH_PAGE_SIZE);
    pm_phys_addr = entry_phys_start;
#ifdef CONFIG_ARCH_HAS_PAGING
    uintptr_t required_pages = required_bytes / ARCH_PAGE_SIZE;
    void *vaddr = mm::vmm::kalloc(required_pages);
    for (uintptr_t i = 0; i < required_pages; i++) {
        uintptr_t virt = ((uintptr_t)vaddr) + (i * ARCH_PAGE_SIZE);
        arch::vmm::set_page(virt, entry_phys_start + (i * ARCH_PAGE_SIZE), arch::vmm::FLAGS_PRESENT);
        arch::vmm::flush_tlb_single(virt);
    }
    pm_areas = (struct area_info *)vaddr;
    pm_bitmap = bitmap(((uint8_t *)vaddr) + (pm_n_areas * sizeof(struct area_info)), ALIGN_UP(pm_n_total_pages, 8) / 8);
#else
    pm_areas = (struct area_info *)entry_phys_start;
    pm_bitmap = bitmap((uint8_t *)entry_phys_start + (pm_n_areas * sizeof(struct area_info)), ALIGN_UP(pm_n_total_pages, 8) / 8);
#endif
}

static void init_pmm_structures() {
    // initialize areas
    struct area_info *area = pm_areas;
    size_t counter = 0;
    const struct mm::mem_map_entry *entry = mm::get_mem_map(counter);
    while (entry != nullptr) {
        if (mm::memmap_is_usable(entry->type) && entry->size > ARCH_PAGE_SIZE) {
            uintptr_t start = ALIGN_UP(entry->base, ARCH_PAGE_SIZE);
            uintptr_t end = ALIGN_DOWN(entry->base + entry->size, ARCH_PAGE_SIZE);
            area->start_addr = start;
            area->n_pages = (end - start) / ARCH_PAGE_SIZE;
            kprintf(KP_INFO, "pmm: area(0x%p) - start: 0x%p n_pages: %u\n", area, area->start_addr, area->n_pages);
            area++;
        }
        counter++;
        entry = mm::get_mem_map(counter);
    }

    // initialize bitmap
    pm_bitmap.clearAll();
}

static size_t find_area_for_paddr(void *paddr) {
    uintptr_t paddr_int = (uintptr_t)paddr;
    for (size_t i = 0; i < pm_n_areas; i++) {
        if (pm_areas[i].start_addr <= paddr_int && ((paddr_int - pm_areas[i].start_addr) / ARCH_PAGE_SIZE) < pm_areas[i].n_pages) {
            return i;
        }
    }
    KERNEL_PANIC("could not find area for paddr");
}

static size_t find_area_for_bitmap_idx(size_t bitmap_idx, size_t *ptr_pages_before = nullptr) {
    size_t pages_before = 0;
    for (size_t i = 0; i < pm_n_areas; i++) {
        if (pages_before >= bitmap_idx && bitmap_idx < (pages_before + pm_areas[i].n_pages)) {
            if (ptr_pages_before != nullptr) {
                *ptr_pages_before = pages_before;
            }
            return i;
        }
        pages_before += pm_areas[i].n_pages;
    }
    KERNEL_PANIC("could not find area for bitmap idx");
}

static size_t find_bitmap_idx_for_paddr(void *paddr, size_t *ptr_pages_before = nullptr) {
    size_t area_idx = find_area_for_paddr(paddr);
    size_t pages_before = 0;
    for (size_t i = 0; i < area_idx; i++) {
        pages_before += pm_areas[i].n_pages;
    }
    uintptr_t pages_offset = ((uintptr_t)paddr - pm_areas[area_idx].start_addr) / ARCH_PAGE_SIZE;
    if (ptr_pages_before != nullptr) {
        *ptr_pages_before = pages_before;
    }
    return pages_before + pages_offset;
}

static void *find_paddr_for_bitmap_idx(size_t bitmap_idx) {
    size_t pages_before;
    size_t area_idx = find_area_for_bitmap_idx(bitmap_idx, &pages_before);
    uintptr_t pages_offset = bitmap_idx - pages_before;
    return (void *)(pm_areas[area_idx].start_addr + (pages_offset * ARCH_PAGE_SIZE));
}

void mm::pmm::init() {
    populate_pmm_info();
    init_pmm_structures();
    // force allocate the data the PMM itself needs
    force_alloc_contiguous((void *)pm_phys_addr,
                           ALIGN_UP((pm_n_areas * sizeof(struct area_info)) + (ALIGN_UP(pm_n_total_pages, 8) / 8), ARCH_PAGE_SIZE) / ARCH_PAGE_SIZE);
    kprintf(KP_INFO, "pmm: initialized\n");
}

status::StatusOr<void *> mm::pmm::alloc_contiguous(size_t pages) {
    size_t pages_before = 0;
    for (size_t i = 0; i < pm_n_areas; i++) {
        size_t found_start;
        bool found = pm_bitmap.findRange(pages_before, pages_before + pm_areas[i].n_pages, pages, false, &found_start);
        if (found) {
            pm_bitmap.setRange(found_start, pages);
            return (void *)(pm_areas[i].start_addr + ((found_start - pages_before) * ARCH_PAGE_SIZE));
        }
        pages_before += pm_areas[i].n_pages;
    }
    return status::StatusCode::OOM_ERROR;
}

void mm::pmm::free_contiguous(void *paddr, size_t pages) {
    size_t idx = find_bitmap_idx_for_paddr(paddr);
    pm_bitmap.setRange(idx, pages);
}

size_t mm::pmm::get_free_blocks() {
    return pm_bitmap.countRange(0, pm_n_total_pages, false);
}

void mm::pmm::force_alloc_contiguous(void *paddr, size_t pages) {
    kprintf(KP_INFO, "pmm: force alloc 0x%p %u pages\n", paddr, pages);
    size_t idx = find_bitmap_idx_for_paddr(paddr);
    pm_bitmap.setRange(idx, pages);
}

void mm::pmm::force_free_contiguous(void *paddr, size_t pages) {
    size_t idx = find_bitmap_idx_for_paddr(paddr);
    pm_bitmap.unsetRange(idx, pages);
}
