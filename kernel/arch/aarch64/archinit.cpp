#include <arch/drivers/gpu/fb.h>
#include <arch/drivers/uart.h>
#include <arch/generic/startup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

#define DEBUG_BUILD

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.putc(c);
}

static void kernelinit() {
#ifdef DEBUG_BUILD
    // cannot use uninitialized uart on real hardware
    drivers::uart::init();
    stdio::set_putc_function(drivers::uart::putc, true);
#endif
    puts("entry\n");
    puts("kernelstart()\n");
    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::generic::startup::stage2_startup() {}

void arch::generic::startup::stage3_startup() {
    struct fb::fbinfo info;
    if (drivers::gpu::setup_fb(1280, 720, 32, &info)) {
        puts("got FB!\n");
        framebuffer.init(info);
        fbconsole.init(&framebuffer);
    }
    fbconsole.init2();
    stdio::set_putc_function(fbputc);
    printf("Hello aarch64!\n");
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::generic::startup::after_init() {
    while (true) {}
}
