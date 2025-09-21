#include <algorithm>
#include <string.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/memmap.h>
#include <vix/panic.h>

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
            map[i].base = ALIGN_UP(map[i].base, CONFIG_ARCH_PAGE_SIZE);
            map[i].size = ALIGN_DOWN(map[i].size, CONFIG_ARCH_PAGE_SIZE);
        }
    }
}
*/

#ifdef CONFIG_ENABLE_MEMMAP_SANITIZE
static void sanitize_usable_entry(
    const struct mm::mem_map_entry *in,
    struct mm::mem_map_entry (*get_entry)(void *ctx, size_t n),
    void *get_entry_ctx,
    bool use_func,
    size_t in_len,
    struct mm::mem_map_entry *out,
    size_t out_len,
    struct mm::mem_map_entry in_ent,
    size_t *out_i
) {
    uint64_t f_base = in_ent.base;
    uint64_t f_size = in_ent.size;
    for (size_t j = 0; j < in_len; j++) {
        struct mm::mem_map_entry in_ent2;
        if (use_func) {
            in_ent2 = get_entry(get_entry_ctx, j);
        } else {
            in_ent2 = in[j];
        }

        if (in_ent2.size == 0 || mm::memmap_is_usable(in_ent2.type)) {
            continue;
        }

        uint64_t end = f_base + f_size;
        uint64_t cmp_end = in_ent2.base + in_ent2.size;

        if (in_ent2.base >= f_base && end > cmp_end) {
            // inside: split into two pieces
            struct mm::mem_map_entry lower = {
                .base = f_base,
                .size = in_ent2.base - f_base,
                .type = in_ent.type,
            };
            struct mm::mem_map_entry upper = {
                .base = cmp_end,
                .size = end - cmp_end,
                .type = in_ent.type,
            };

            if (lower.size > 0) {
                sanitize_usable_entry(
                    in,
                    get_entry,
                    get_entry_ctx,
                    use_func,
                    in_len,
                    out,
                    out_len,
                    lower,
                    out_i
                );
            }

            if (upper.size > 0) {
                sanitize_usable_entry(
                    in,
                    get_entry,
                    get_entry_ctx,
                    use_func,
                    in_len,
                    out,
                    out_len,
                    upper,
                    out_i
                );
            }

            return; // stop processing current entry (it's been split up and added seperately so..)
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
    if (in_ent.size) {
        if (*out_i >= out_len) {
            KERNEL_PANIC("ran out of memory map space, try increasing CONFIG_MEMMAP_MAX_ENTRIES");
            return;
        }
        out[*out_i].base = f_base;
        out[*out_i].size = f_size;
        out[*out_i].type = in_ent.type;
        (*out_i)++;
    }
}

static void sanitize(
    const struct mm::mem_map_entry *in,
    struct mm::mem_map_entry (*get_entry)(void *ctx, size_t n),
    void *get_entry_ctx,
    bool use_func,
    size_t in_len,
    struct mm::mem_map_entry *out,
    size_t out_len
) {
    size_t out_i = 0;
    if (out_i >= out_len) {
        KERNEL_PANIC("ran out of memory map space, try increasing CONFIG_MEMMAP_MAX_ENTRIES");
        return;
    }
    for (size_t i = 0; i < in_len; i++) {
        struct mm::mem_map_entry in_ent;
        if (use_func) {
            in_ent = get_entry(get_entry_ctx, i);
        } else {
            in_ent = in[i];
        }
        if (in_ent.size == 0) {
            continue;
        }

        if (mm::memmap_is_usable(in_ent.type)) {
            sanitize_usable_entry(in, get_entry, get_entry_ctx, use_func, in_len, out, out_len, in_ent, &out_i);
        } else {
            // just add non-usable entries unchanged
            if (in_ent.size) {
                if (out_i >= out_len) {
                    KERNEL_PANIC("ran out of memory map space, try increasing CONFIG_MEMMAP_MAX_ENTRIES");
                    return;
                }
                out[out_i].base = in_ent.base;
                out[out_i].size = in_ent.size;
                out[out_i].type = in_ent.type;
                out_i++;
            }
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
#endif // CONFIG_ENABLE_MEMMAP_SANITIZE

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
    kprintf(KP_INFO, "final memory map:\n");
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
    if (len > CONFIG_MEMMAP_MAX_ENTRIES) {
        KERNEL_PANIC("too many memory map entries. There is space for %u but we got %u", CONFIG_MEMMAP_MAX_ENTRIES, len);
    }
    memset(memory_map, 0, CONFIG_MEMMAP_MAX_ENTRIES * sizeof(struct mm::mem_map_entry));
#ifdef CONFIG_ENABLE_MEMMAP_SANITIZE
    for (size_t i = 0; i < len; i++) {
        if (in[i].size == 0) {
            continue;
        }
        kprintf(KP_INFO, "memmap(unsanitized): [mem 0x%p l: %u] %s\n", (uintptr_t)in[i].base, (uintptr_t)in[i].size, type_string(in[i].type));
    }
    sanitize(in, nullptr, nullptr, false, len, memory_map, CONFIG_MEMMAP_MAX_ENTRIES);
#else
    memcpy(memory_map, in, len * sizeof(struct mm::mem_map_entry));
#endif
    init_map();
}

void mm::set_mem_map(struct mem_map_entry (*get_entry)(void *ctx, size_t n), size_t len, void *get_entry_ctx) {
    if (len > CONFIG_MEMMAP_MAX_ENTRIES) {
        KERNEL_PANIC("too many memory map entries. There is space for %u but we got %u", CONFIG_MEMMAP_MAX_ENTRIES, len);
    }
    memset(memory_map, 0, CONFIG_MEMMAP_MAX_ENTRIES * sizeof(struct mm::mem_map_entry));
#ifdef CONFIG_ENABLE_MEMMAP_SANITIZE
    for (size_t i = 0; i < len; i++) {
        struct mem_map_entry e = get_entry(get_entry_ctx, i);
        if (e.size == 0) {
            continue;
        }
        kprintf(KP_INFO, "memmap(unsanitized): [mem 0x%p l: %u] %s\n", (uintptr_t)e.base, (uintptr_t)e.size, type_string(e.type));
    }
    sanitize(nullptr, get_entry, get_entry_ctx, true, len, memory_map, CONFIG_MEMMAP_MAX_ENTRIES);
#else
    for (size_t i = 0; i < len; i++) {
        memory_map[i] = get_entry(get_entry_ctx, i);
    }
#endif
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
