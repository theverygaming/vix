#include "debug.h"
#include "../config.h"
#include "drivers/serial.h"
#include "paging.h"
#include "stdio.h"

void debug::stack_trace(uint32_t maxLength, uint32_t ebp) {
    printf("Stack trace:\n");
    for (uint32_t i = 0; ebp && i < maxLength; i++) {
        if (!paging::is_readable((void *)ebp) || !paging::is_readable((void *)ebp + 4)) {
            break;
        }
        uint32_t eip = *((uint32_t *)ebp + 4);
        printf("->0x%p\n", eip);
        ebp = *((uint32_t *)ebp);
    }
}

void debug::debug_loop() {
    printf("DETH\n");
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            putcolor(x, y, 0x1F);
        }
    }

    while (true)
        ;
}

void debug::breakpoint(int line, char *file) {
    printf_serial("Breakpoint hit in %s at line %d\nPress any key to continue\n", file, line);
    drivers::serial::getc();
}

void debug::visual_dbg(char c) {
    *((char *)((KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET) + 2 * 70 + 160 * 0)) = c;
    for (uint32_t i = 0; i < 0xFFFFFFF; i++) {}
}