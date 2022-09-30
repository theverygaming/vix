#include <arch/x86/gdt.h>
#include <config.h>
#include <log.h>
#include <stdlib.h>
#include <types.h>

struct __attribute__((packed)) gdtEntry {
    uint16_t limitLow;       // limit (bits 0-15)
    uint16_t baseLow;        // base (bits 0-15)
    uint8_t baseMiddle;      // base (bits 16-23)
    uint8_t access;          // access
    uint8_t limitHigh_flags; // limit (bits 16-19) | flags
    uint8_t baseHigh;        // base (bits 24-31)
};

struct __attribute__((packed)) gdtDescriptor {
    uint16_t Limit; // sizeof(gdt) - 1
    struct gdtEntry *offset;
};

enum gdt_access {
    GDT_ACCESS_CODE_READABLE = 0x02,
    GDT_ACCESS_DATA_WRITEABLE = 0x02,

    GDT_ACCESS_CODE_CONFORMING = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN = 0x04,

    GDT_ACCESS_DATA_SEGMENT = 0x10,
    GDT_ACCESS_CODE_SEGMENT = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS = 0x00,

    GDT_ACCESS_RING0 = 0x00,
    GDT_ACCESS_RING1 = 0x20,
    GDT_ACCESS_RING2 = 0x40,
    GDT_ACCESS_RING3 = 0x60,

    GDT_ACCESS_PRESENT = 0x80,
};

enum gdt_flags {
    GDT_FLAG_16BIT = 0x0, // no flags set
    GDT_FLAG_32BIT = 0x4, // size flag set(32-bit protected mode segment)
    GDT_FLAG_64BIT = 0x2, // long mode flag set

    GDT_FLAG_GRANULARITY_1B = 0x0,
    GDT_FLAG_GRANULARITY_4K = 0x8,
};

static struct gdtEntry make_gdt_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    struct gdtEntry entry;
    stdlib::memset(&entry, 0, sizeof(struct gdtEntry));

    entry.baseLow = base & 0xFFFF;
    entry.baseMiddle = (base >> 16) & 0xFF;
    entry.baseHigh = (base >> 24) & 0xFF;

    entry.limitLow = limit & 0xFFFF;
    entry.limitHigh_flags = (limit >> 16) & 0xF; // limit is 20 bits

    entry.limitHigh_flags |= (flags << 4) & 0xF0;

    entry.access = access;

    return entry;
}

static gdtEntry gdtTable[3];

extern "C" void GDT_load_32(struct gdtDescriptor *descriptor, uint16_t codeSegment, uint16_t dataSegment);

void gdt::init() {
    gdtTable[0] = make_gdt_entry(0, 0, 0, 0); // NULL descriptor
    gdtTable[1] = make_gdt_entry(0, 0xFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
    gdtTable[2] = make_gdt_entry(0, 0xFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K);
    struct gdtDescriptor descriptor = {sizeof(gdtTable) - 1, gdtTable};
    GDT_load_32(&descriptor, i686_GDT_CODE_SEGMENT, i686_GDT_DATA_SEGMENT);
    log::log_service("GDT", "initialized");
}
