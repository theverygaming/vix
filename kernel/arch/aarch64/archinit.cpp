#include <arch/drivers/gpu/fb.h>
#include <arch/drivers/timer.h>
#include <arch/drivers/uart.h>
#include <arch/generic/startup.h>
#include <arch/limine.h>
#include <arch/startup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <mm/kmalloc.h>
#include <mm/memmap.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

static fb::fb framebuffer;
static fb::fbconsole fbconsole;

static volatile uint8_t static_alloc_mem[4096 * 1000];

static volatile struct limine_terminal_request terminal_request = {.id = LIMINE_TERMINAL_REQUEST, .revision = 0};

static void limineputc(char c) {
    struct limine_terminal *terminal = terminal_request.response->terminals[0];
    terminal_request.response->write(terminal, &c, 1);
}

static volatile bool done = false;
static void cpuinit(uint64_t cpu) {
    while (done) {}
    printf("i am CPU %u\n", cpu);
    done = true;
    while (true) {}
}

static void kernelinit() {
    if (terminal_request.response == NULL || terminal_request.response->terminal_count < 1) {
        while (true) {}
    }
    stdio::set_putc_function(limineputc, true);
    puts("entry\n");
    // puts("enabling timer\n");
    // drivers::timer::init();
    puts("kernelstart()\n");

    struct mm::mem_map_entry r;
    r.type = mm::mem_map_entry::type_t::RAM;
    r.base = (uintptr_t)static_alloc_mem;
    r.size = sizeof(static_alloc_mem) / sizeof(static_alloc_mem[0]);
    mm::set_mem_map(&r, 1);

    kernelstart();
}

extern "C" void _kentry() {
    kernelinit();
    while (true) {}
}

void arch::generic::startup::stage2_startup() {}

void arch::generic::startup::stage3_startup() {
    /*struct fb::fbinfo info;
    if (drivers::gpu::setup_fb(1280, 720, 32, &info)) {
        puts("got FB!\n");
        framebuffer.init(info);
        fbconsole.init(&framebuffer);
    }
    fbconsole.init2();*/
    // stdio::set_putc_function(fbputc);
    printf("Hello aarch64!\n");
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::generic::startup::after_init() {}
