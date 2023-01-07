#include <arch/generic/startup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void idkputc(char c) {
    fbconsole.putc(c);
}

#include <arch/drivers/uart.h>

static void kernelinit() {
    //stdio::set_putc_function(drivers::uart::putc);
    stdio::set_putc_function(drivers::uart::putc, true);
    puts("testing printf\n");
    printf("hello kernel!\n");
    puts("tested printf\n");
    // framebuffer.init(multiboot2::findFrameBuffer(multiboot2_info_ptr));
    //fbconsole.init(&framebuffer);
    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::generic::startup::stage2_startup() {}

void arch::generic::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::generic::startup::after_init() {
    framebuffer.clear();
    fbconsole.init2();
    // stdio::set_putc_function(idkputc);

    while (true) {}
}
