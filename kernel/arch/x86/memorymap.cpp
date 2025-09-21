#include <string.h>
#include <vix/arch/memorymap.h>
#include <vix/debug.h>
#include <vix/kprintf.h>
#include <vix/mm/memmap.h>
#include <vix/panic.h>

struct __attribute__((packed)) e820_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

static mm::mem_map_entry::type_t convert_from_e820(uint32_t e820_type) {
    switch (e820_type) {
    case 1:
        return mm::mem_map_entry::type_t::RAM;
    case 2:
        return mm::mem_map_entry::type_t::RESERVED;
    case 3:
        return mm::mem_map_entry::type_t::ACPI_RECLAIM;
    case 4:
        return mm::mem_map_entry::type_t::ACPI_NVS;
    case 5:
        return mm::mem_map_entry::type_t::UNUSABLE;
    case 6:
        return mm::mem_map_entry::type_t::DISABLED;
    case 7:
        return mm::mem_map_entry::type_t::PERSISTENT;
    default:
        return mm::mem_map_entry::type_t::UNKNOWN;
    }
}

void memorymap::initMemoryMap(void *mapadr, size_t entrycount, struct mm::mem_map_entry (*extra_entries)(size_t n), size_t extra_entry_count) {
    struct set_mem_map_ctx {
        size_t entrycount;
        struct mm::mem_map_entry (*extra_entries)(size_t n);
        struct e820_entry *entries;
    };

    struct set_mem_map_ctx extra_entries_ctx = {
        .entrycount = entrycount,
        .extra_entries = extra_entries,
        .entries = (struct e820_entry *)mapadr,
    };

    mm::set_mem_map(
        [](void *ctx, size_t n) -> struct mm::mem_map_entry {
            struct set_mem_map_ctx *c = (struct set_mem_map_ctx *)ctx;
            if (n >= c->entrycount) {
                return c->extra_entries(n - c->entrycount);
            }

            struct mm::mem_map_entry r = {
                .base = c->entries[n].base,
                .size = c->entries[n].length,
                .type = convert_from_e820(c->entries[n].type),
            };

            return r;
        },
        entrycount + extra_entry_count,
        &extra_entries_ctx
    );
}
