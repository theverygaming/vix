#include <arch/drivers/gpu/fb.h>
#include <arch/drivers/timer.h>
#include <arch/drivers/uart.h>
#include <arch/generic/startup.h>
#include <arch/startup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

#define DEBUG_BUILD

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static void fbputc(char c) {
    fbconsole.putc(c);
}

static volatile bool done = false;
static void cpuinit(uint64_t cpu) {
    _initcpuid = 0;
    printf("i am CPU %u\n", cpu);
    done = true;
    while (true) {}
}

static void kernelinit() {
#ifdef DEBUG_BUILD
    // cannot use uninitialized uart on real hardware
    drivers::uart::init();
    stdio::set_putc_function(drivers::uart::putc, true);
#endif
    puts("entry\n");
    // puts("enabling timer\n");
    // drivers::timer::init();
    puts("kernelstart()\n");
    kernelstart();
}

extern "C" void _kentry(uint64_t mpidr_el1) {
    if (mpidr_el1 & 3) {
        // while (true) {}
    }
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

extern "C" void _start();

void arch::generic::startup::after_init() {
    // wake up cores (addresses from https://wiki.osdev.org/Raspberry_Pi_Bare_Bones)
    *((volatile uint64_t *)0xE0) = (uint64_t)_start;
    asm volatile("sev");
    *((volatile uint64_t *)0xE8) = (uint64_t)_start;
    asm volatile("sev");
    *((volatile uint64_t *)0xF0) = (uint64_t)_start;
    asm volatile("sev");

    _initcpuadr = (uint64_t)cpuinit;
    for (int i = 1; i <= 3; i++) {
        printf("inititalizing CPU %d\n", i);
        _initcpustack = (uint64_t)mm::kmalloc_aligned(1000, 8);
        _initcpuid = i;
        asm volatile("sev");
        while (_initcpuid != 0) {}
        while (!done) {}
        done = false;
    }
    while (true) {}
}
