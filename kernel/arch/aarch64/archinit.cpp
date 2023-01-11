#include <arch/drivers/gpu/fb.h>
#include <arch/drivers/uart.h>
#include <arch/generic/startup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.putc(c);
}

static void kernelinit() {
    stdio::set_putc_function(drivers::uart::putc, true);
    puts("entry\n");
    struct fb::fbinfo info;
    if (drivers::gpu::setup_fb(1920, 1080, 32, &info)) {
        puts("got FB!\n");
        framebuffer.init(info);
        fbconsole.init(&framebuffer);
    }
    puts("kernelstart()\n");
    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::generic::startup::stage2_startup() {}

void arch::generic::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
    fbconsole.init2();
    stdio::set_putc_function(fbputc);
    printf("Hello aarch64!\n");
}

void arch::generic::startup::after_init() {
    while (true) {}
}
