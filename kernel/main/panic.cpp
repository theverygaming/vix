#include "panic.h"
#include <stdio.h>

void panic::kernel_panic(const char *file, int line, const char *text) {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            putcolor(x, y, 0x4F);
        }
    }
    printf("kernel panic in %s at line %d\n    -> %s\n", file, line, text);

    while (true) {
        asm volatile("cli");
        asm volatile("hlt");
    }
}