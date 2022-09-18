#include "../../kernel/include/arch/x86/archspecific.h"
#include <gdt.h>
#include <paging.h>
#include <stdlib.h>

extern "C" void puts_c(const char *str);
extern "C" void startup_cpp() {
    puts_c("loaded C++\n\r");
    memcpy((char *)KERNEL_PHYS_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET, (char *)KERNEL_PHYS_ADDRESS + KERNEL_CODE_SIZE, 40 * 65536); // copy ramfs to the right location
    paging::initpaging();
    puts_c("paging init\n\r");
    gdt::i686_GDT_Initialize();
    puts_c("GDT init\n\r");
}
