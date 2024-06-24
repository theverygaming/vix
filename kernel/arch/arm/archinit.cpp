#include <arch/common/bootup.h>
#include <config.h>
#include <framebuffer.h>
#include <kernel.h>
#include <mm/kmalloc.h>
#include <mm/memmap.h>
#include <panic.h>
#include <stdio.h>

static volatile uint8_t static_alloc_mem[4096 * 1000];

static void kernelinit() {
    stdio::set_putc_function([](char c) {}, true);
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

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {
    printf("Hello arm!\n");
}

void arch::startup::kthread0() {}
