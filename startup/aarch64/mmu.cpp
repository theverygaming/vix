#include <memmap.h>
#include <mmu.h>

struct entry_stage1_4KiB {
    // VMSAv8-64
    bool not_invalid;  // b0
    bool not_reserved; // b1
    // lower attributes -- described on page 2595 of armv8 reference manual
    bool lower_AttrIndx[2];
    bool lower_NS;
    bool lower_AP[2]; // https://developer.arm.com/documentation/102376/0100/Permissions-attributes
    bool lower_SH[2]; // https://developer.arm.com/documentation/ddi0406/cb/System-Level-Architecture/Virtual-Memory-System-Architecture--VMSA-/Memory-region-attributes/Long-descriptor-format-memory-region-attributes
    bool lower_AF;
    bool lower_nG;
    bool lower_nT;

    uint64_t address;

    // upper attributes
    bool upper_GP;
    bool upper_DBM;
    bool upper_Contiguous;
    bool upper_PXN;
    bool upper_XN;
    // PBHA implementation defined stuff after
};

static uint64_t make_entry(struct entry_stage1_4KiB e) {
    // 4KiB page - stage 1
    uint64_t value = 0;
    value |= (e.not_invalid & 0x1) << 0;
    value |= (e.not_reserved & 0x1) << 1;
    // lower attributes
    value |= (e.lower_AttrIndx[0] & 0x1) << 2;
    value |= (e.lower_AttrIndx[1] & 0x1) << 3;
    value |= (e.lower_NS & 0x1) << 4;
    value |= (e.lower_AP[0] & 0x1) << 5;
    value |= (e.lower_AP[1] & 0x1) << 6;
    value |= (e.lower_SH[0] & 0x1) << 7;
    value |= (e.lower_SH[1] & 0x1) << 8;
    value |= (e.lower_AF & 0x1) << 9;
    value |= (e.lower_nG & 0x1) << 10;
    // value |= (e.lower_nT & 0x1) << 15; ???? sets a bit in the address?

    // address
    value |= e.address & 0xFFFFFFFFF000;

    // upper attributes
    value |= (e.upper_GP & 0x1) << 49;
    value |= (e.upper_DBM & 0x1) << 50;
    value |= (e.upper_Contiguous & 0x1) << 51;
    value |= (e.upper_PXN & 0x1) << 52;
    value |= (e.upper_XN & 0x1) << 53;
}

void test_mmu() {}
