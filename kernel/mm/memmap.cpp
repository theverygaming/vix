#include <algorithm>
#include <arch/generic/memory.h>
#include <config.h>
#include <kprintf.h>
#include <macros.h>
#include <mm/memmap.h>
#include <panic.h>
#include <string.h>

static const char *type_string(mm::mem_map_entry::type_t type) {
    switch (type) {
    case mm::mem_map_entry::type_t::NONE:
        return "none";
    case mm::mem_map_entry::type_t::RAM:
        return "usable";
    case mm::mem_map_entry::type_t::RECLAIMABLE:
        return "reclaimable";
    case mm::mem_map_entry::type_t::RESERVED:
        return "system reserved";
    case mm::mem_map_entry::type_t::ACPI_RECLAIM:
        return "ACPI reclaimable";
    case mm::mem_map_entry::type_t::ACPI_NVS:
        return "ACPI NVS";
    case mm::mem_map_entry::type_t::UNUSABLE:
        return "unusable";
    case mm::mem_map_entry::type_t::DISABLED:
        return "disabled";
    case mm::mem_map_entry::type_t::PERSISTENT:
        return "persistent";
    case mm::mem_map_entry::type_t::UNKNOWN:
        return "unknown";
    default:
        return "";
    }
}

/*
static void align_map(struct mm::mem_map_entry *map, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (mm::memmap_is_usable(map[i].type)) {
            map[i].base = ALIGN_UP(map[i].base, ARCH_PAGE_SIZE);
            map[i].size = ALIGN_DOWN(map[i].size, ARCH_PAGE_SIZE);
        }
    }
}
*/

static void sanitize(const struct mm::mem_map_entry *in,
                     struct mm::mem_map_entry (*get_entry)(size_t n),
                     bool use_func,
                     size_t in_len,
                     struct mm::mem_map_entry *out,
                     size_t out_len) {
    size_t out_i = 0;
    if (out_i >= out_len) {
        KERNEL_PANIC("ran out of memory map space, try increasing CONFIG_MEMMAP_MAX_ENTRIES");
        return;
    }
    for (size_t i = 0; i < in_len; i++) {
        struct mm::mem_map_entry in_ent;
        if (use_func) {
            in_ent = get_entry(i);
        } else {
            in_ent = in[i];
        }
        if (in_ent.size == 0) {
            continue;
        }

        uint64_t f_base = in_ent.base;
        uint64_t f_size = in_ent.size;

        if (mm::memmap_is_usable(in_ent.type)) {
            for (size_t j = 0; j < in_len; j++) {

                struct mm::mem_map_entry in_ent2;
                if (use_func) {
                    in_ent2 = get_entry(j);
                } else {
                    in_ent2 = in[j];
                }

                if (i == j || in_ent2.size == 0 || mm::memmap_is_usable(in_ent2.type)) {
                    continue;
                }

                uint64_t end = f_base + f_size;
                uint64_t cmp_end = in_ent2.base + in_ent2.size;

                if (in_ent2.base >= f_base && end > cmp_end) {
                    // inside
                    KERNEL_PANIC("unsupported: usable memory mab entry inside free entry");
                    return;
                } else if (in_ent2.base <= f_base && cmp_end > end) {
                    // full overlap
                    f_size = 0;
                    break;
                } else if (cmp_end > end && in_ent2.base > f_base && in_ent2.base < end) {
                    // above
                    f_size = in_ent2.base - f_base;
                } else if (cmp_end > f_base && in_ent2.base <= f_base) {
                    // below/equal
                    f_base = cmp_end;
                    f_size = end - f_base;
                }
            }
        }

        if (f_size != 0) {
            if (out_i >= out_len) {
                KERNEL_PANIC("ran out of memory map space, try increasing CONFIG_MEMMAP_MAX_ENTRIES");
                return;
            }
            out[out_i].base = f_base;
            out[out_i].size = f_size;
            out[out_i].type = in_ent.type;
            out_i++;
        }
    }

    // fix free overlaps
    for (size_t i = 0; i < out_i; i++) {
        if (!mm::memmap_is_usable(out[i].type) || out[i].size == 0) {
            continue;
        }

        uint64_t f_base = out[i].base;
        uint64_t f_size = out[i].size;

        for (size_t j = 0; j < out_i; j++) {
            if (!mm::memmap_is_usable(out[j].type) || out[j].size == 0 || i == j) {
                continue;
            }

            uint64_t end = f_base + f_size;
            uint64_t cmp_end = out[j].base + out[j].size;

            if (out[j].base >= f_base && end > cmp_end) {
                // inside
                out[j].size = 0;
            } else if (out[j].base <= f_base && cmp_end > end) {
                // full overlap
                f_size = 0;
                break;
            } else if (cmp_end > end && out[j].base > f_base && out[j].base < end) {
                // above
                f_size = cmp_end - f_base;
                out[j].size = 0;
            } else if (cmp_end > f_base && out[j].base <= f_base) {
                // below/equal
                f_base = out[j].base;
                f_size = end - f_base;
            } else if (end == out[j].base) {
                // cmp after
                f_size = cmp_end - f_base;
                out[j].size = 0;
            } else if (cmp_end == f_base) {
                // cmp before
                f_base = out[j].base;
                f_size = end - f_base;
                out[j].size = 0;
            }
        }

        out[i].base = f_base;
        out[i].size = f_size;
    }
}

static void print_map(const struct mm::mem_map_entry *map, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (map[i].size == 0) {
            continue;
        }
        kprintf(KP_INFO, "memmap: [mem 0x%p l: %u] %s\n", (uintptr_t)map[i].base, (uintptr_t)map[i].size, type_string(map[i].type));
    }
}

static struct mm::mem_map_entry memory_map[CONFIG_MEMMAP_MAX_ENTRIES];

static void init_map() {
    std::bubblesort(
        memory_map, memory_map + CONFIG_MEMMAP_MAX_ENTRIES, [](const struct mm::mem_map_entry a, const struct mm::mem_map_entry b) -> bool {
            if (a.base == b.base) {
                return a.size < b.size;
            }
            return a.base < b.base;
        });
    kprintf(KP_INFO, "sanitized memory map:\n");
    print_map(memory_map, CONFIG_MEMMAP_MAX_ENTRIES);
    uint64_t total = 0;
    for (size_t i = 0; i < CONFIG_MEMMAP_MAX_ENTRIES; i++) {
        if (mm::memmap_is_usable(memory_map[i].type)) {
            total += memory_map[i].size;
        }
    }
    total /= 1048576;
    kprintf(KP_INFO, "total usable memory: %uMiB\n", (uintptr_t)total);
}

void mm::set_mem_map(const struct mem_map_entry *in, size_t len) {
    memset(memory_map, 0, CONFIG_MEMMAP_MAX_ENTRIES * sizeof(struct mm::mem_map_entry));
    sanitize(in, nullptr, false, len, memory_map, CONFIG_MEMMAP_MAX_ENTRIES);
    init_map();
}

void mm::set_mem_map(struct mem_map_entry (*get_entry)(size_t n), size_t len) {
    memset(memory_map, 0, CONFIG_MEMMAP_MAX_ENTRIES * sizeof(struct mm::mem_map_entry));
    sanitize(nullptr, get_entry, true, len, memory_map, CONFIG_MEMMAP_MAX_ENTRIES);
    init_map();
}

bool mm::memmap_is_usable(mm::mem_map_entry::type_t type) {
    return type == mm::mem_map_entry::type_t::RAM;
}

const struct mm::mem_map_entry *mm::get_mem_map(size_t index) {
    if (index >= CONFIG_MEMMAP_MAX_ENTRIES) {
        return nullptr;
    }
    return &memory_map[index];
}

uint64_t mm::mem_map_get_total_usable_bytes() {
    uint64_t total = 0;
    size_t i = 0;
    const mem_map_entry *e;
    while ((e = mm::get_mem_map(i++)) != nullptr) {
        if (memmap_is_usable(e->type)) {
            total += e->size;
        }
    }
    return total;
}
