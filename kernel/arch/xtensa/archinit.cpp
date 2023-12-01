#include <arch/common/bootup.h>
#include <arch/generic/memory.h>
#include <config.h>
#include <kernel.h>
#include <kprintf.h>
#include <macros.h>
#include <mm/kmalloc.h>
#include <mm/memmap.h>
#include <mm/memtest.h>
#include <mm/phys.h>
#include <panic.h>
#include <stdio.h>
#include <time.h>

// ESP32 ROM
void (*ets_write_char_uart)(char c) = (void (*)(char))0x40007cf8;

static void kernelinit() {

    stdio::set_putc_function(ets_write_char_uart, true);
    kernelstart();
}

extern "C" uint8_t __bss_start;
extern "C" uint8_t __bss_end;

extern "C" void _kentry() {
    for (uint8_t *addr = &__bss_start; addr < &__bss_end; addr++) {
        *addr = 0;
    }
    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::startup::kthread0() {}
