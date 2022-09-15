#include <kernel.h>

static void kernelinit() {
    kernelstart(); // kernelstart must be called
}

extern "C" void __attribute__((section(".entry"))) _kentry() {
    kernelinit();
    while (true) {}
}
