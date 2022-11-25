#include <memmap.h>
#include <mmu.h>

struct stage1_lower_attributes {
    // lower attributes -- described on page 2595 of armv8 reference manual
    bool AttrIndx[2];
    bool NS;
    bool AP[2]; // https://developer.arm.com/documentation/102376/0100/Permissions-attributes
    bool SH[2]; // https://developer.arm.com/documentation/ddi0406/cb/System-Level-Architecture/Virtual-Memory-System-Architecture--VMSA-/Memory-region-attributes/Long-descriptor-format-memory-region-attributes
    bool AF;
    bool nG;
    bool nT;
};

struct stage1_upper_attributes {
    // upper attributes
    bool GP;
    bool DBM;
    bool Contiguous;
    bool PXN;
    bool XN;
    // PBHA implementation defined stuff after
};

struct descriptor_stage1_block {
    enum class granule { GRANULE_4KiB, GRANULE_16KiB, GRANULE_64KiB } granule;
    enum class level { LEVEL_1, LEVEL_2, LEVEL_3 } level;

    struct stage1_lower_attributes lower;

    uint64_t address;

    struct stage1_upper_attributes upper;
};

struct descriptor_stage1_table {
    enum class granule { GRANULE_4KiB, GRANULE_16KiB, GRANULE_64KiB } granule;

    uint64_t address;

    // table attributes
    bool PXNTable;
    bool XNTable;
    bool APTable[2]; // -> page 2593 of armv8 reference manual
    bool NSTable;
};

static uint64_t make_entry_new(struct descriptor_stage1_table e) {
    uint64_t value = 0;
    value |= (uint64_t)(1 & 0x1) << 0;
    value |= (uint64_t)(1 & 0x1) << 1;

    // address
    switch (e.granule) {
    case descriptor_stage1_table::granule::GRANULE_4KiB:
        value |= e.address & 0xFFFFFFFFF000;
        break;
    case descriptor_stage1_table::granule::GRANULE_16KiB:
        value |= e.address & 0xFFFFFFFFC000;
        break;
    case descriptor_stage1_table::granule::GRANULE_64KiB:
        value |= e.address & 0xFFFFFFFF0000;
        break;
    }

    value |= (uint64_t)(e.PXNTable & 0x1) << 59;
    value |= (uint64_t)(e.XNTable & 0x1) << 60;
    value |= (uint64_t)(e.APTable[0] & 0x1) << 61;
    value |= (uint64_t)(e.APTable[1] & 0x1) << 62;
    value |= (uint64_t)(e.NSTable & 0x1) << 63;
    return value;
}

static uint64_t make_entry_new(struct descriptor_stage1_block e) {
    uint64_t value = 0;
    value |= (uint64_t)(1 & 0x1) << 0;
    if (e.level == descriptor_stage1_block::level::LEVEL_3) {
        value |= (uint64_t)(1 & 0x1) << 1;
    }

    // lower attributes
    value |= (uint64_t)(e.lower.AttrIndx[0] & 0x1) << 2;
    value |= (uint64_t)(e.lower.AttrIndx[1] & 0x1) << 3;
    value |= (uint64_t)(e.lower.NS & 0x1) << 4;
    value |= (uint64_t)(e.lower.AP[0] & 0x1) << 5;
    value |= (uint64_t)(e.lower.AP[1] & 0x1) << 6;
    value |= (uint64_t)(e.lower.SH[0] & 0x1) << 7;
    value |= (uint64_t)(e.lower.SH[1] & 0x1) << 8;
    value |= (uint64_t)(e.lower.AF & 0x1) << 9;
    value |= (uint64_t)(e.lower.nG & 0x1) << 10;
    if (e.level != descriptor_stage1_block::level::LEVEL_3) {
        value |= (uint64_t)(e.lower.nT & 0x1) << 15;
    }

    // address
    switch (e.granule) { // https://developer.arm.com/documentation/den0024/a/The-Memory-Management-Unit/Translation-tables-in-ARMv8-A/Effect-of-granule-sizes-on-translation-tables
    case descriptor_stage1_block::granule::GRANULE_4KiB: // +- 9 bits
        if (e.level == descriptor_stage1_block::level::LEVEL_1) {
            value |= e.address & 0xFFFFC0000000; // n = 30
        } else if (e.level == descriptor_stage1_block::level::LEVEL_2) {
            value |= e.address & 0xFFFFFFE00000; // n = 21
        } else if (e.level == descriptor_stage1_block::level::LEVEL_3) {
            value |= e.address & 0xFFFFFFFFF000; // n = 12
        }
        break;
    case descriptor_stage1_block::granule::GRANULE_16KiB: // +- 11 bits
        if (e.level == descriptor_stage1_block::level::LEVEL_1) {
            value |= e.address & 0xFFF000000000; // n = 36
        } else if (e.level == descriptor_stage1_block::level::LEVEL_2) {
            value |= e.address & 0xFFFFFE000000; // n = 25
        } else if (e.level == descriptor_stage1_block::level::LEVEL_3) {
            value |= e.address & 0xFFFFFFFFC000; // n = 14
        }
        break;
    case descriptor_stage1_block::granule::GRANULE_64KiB: // +- 13 bits
        if (e.level == descriptor_stage1_block::level::LEVEL_1) {
            value |= e.address & 0xFC0000000000; // n = 42
        } else if (e.level == descriptor_stage1_block::level::LEVEL_2) {
            value |= e.address & 0xFFFFE0000000; // n = 29
        } else if (e.level == descriptor_stage1_block::level::LEVEL_3) {
            value |= e.address & 0xFFFFFFFF0000; // n = 16
        }
        break;
    }

    // upper attributes
    value |= (uint64_t)(e.upper.GP & 0x1) << 49;
    value |= (uint64_t)(e.upper.DBM & 0x1) << 50;
    value |= (uint64_t)(e.upper.Contiguous & 0x1) << 51;
    value |= (uint64_t)(e.upper.PXN & 0x1) << 52;
    value |= (uint64_t)(e.upper.XN & 0x1) << 53;
    return value;
}

void test_mmu() {
    
}
