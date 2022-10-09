#pragma once

#define i686_GDT_CODE_SEGMENT (0x08 * 1) // specifies offset in bytes in GDT
#define i686_GDT_DATA_SEGMENT (0x08 * 2)

#define i686_USER_CODE_SEGMENT ((0x08 * 3) | 3)
#define i686_USER_DATA_SEGMENT ((0x08 * 4) | 3)

namespace gdt {
    void init();
}
