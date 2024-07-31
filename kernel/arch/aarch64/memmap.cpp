#include <vix/arch/memmap.h>

// BCM2837 rpi3b

int memory_map_entrycount = 3;

memmap_entry_t memory_map[] = {
    {0x0, 0x40000000, memory_type::MEM_RAM},
    {0x20000000, 0xFFFFFF, memory_type::MEM_DEVICE},       // peripherals from 0x7E000000
    {0x3D090000, 32000000, memory_type::MEM_RAM_UNUSABLE}, // GPU memory(32mb min.)
};
