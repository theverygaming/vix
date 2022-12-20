#pragma once
#include <types.h>

#define i686_GDT_CODE_SEGMENT (0x08 * 1) // specifies offset in bytes in GDT
#define i686_GDT_DATA_SEGMENT (0x08 * 2)

#define i686_USER_CODE_SEGMENT ((0x08 * 3) | 3)
#define i686_USER_DATA_SEGMENT ((0x08 * 4) | 3)

namespace gdt {
    void init();

    void set_ldt_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
    void set_tls_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
}
