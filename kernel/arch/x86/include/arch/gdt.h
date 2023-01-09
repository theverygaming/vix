#pragma once
#include <types.h>

#define GDT_KERNEL_CODE (1 * 8)
#define GDT_KERNEL_DATA (2 * 8)
#define GDT_USER_CODE   (3 * 8)
#define GDT_USER_DATA   (4 * 8)

namespace gdt {
    void init();

    void set_ldt_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
    void set_tls_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
}
