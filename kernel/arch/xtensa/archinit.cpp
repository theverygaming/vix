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

extern "C" uint8_t __bss_start;
extern "C" uint8_t __bss_end;

static void kernelinit() {
    stdio::set_putc_function(ets_write_char_uart, true);
    mm::set_mem_map(
        [](size_t n) -> struct mm::mem_map_entry {
            struct mm::mem_map_entry r;

            r.base = (uintptr_t)&__bss_end;
            r.size = (0x3FFF0000 + 0x70001) - (uintptr_t)&__bss_end;
            r.type = mm::mem_map_entry::type_t::RAM;

            return r;
        },
        1);
    kernelstart();
}

inline void write_addr_32(uint32_t *addr, uint32_t value) {
    volatile uint32_t *ptr = (volatile uint32_t *)addr;
    *ptr = value;
}

extern "C" void _kentry() {
    for (uint8_t *addr = &__bss_start; addr < &__bss_end; addr++) {
        *addr = 0;
    }

    // HACK: disable the WDT's
    // RTC WDT
    write_addr_32((uint32_t *)0x3FF480A4, 0x050D83AA1); // RTC_CNTL_WDTWPROTECT_REG
    write_addr_32((uint32_t *)0x3FF4808C, 0x00);        // RTC_CNTL_WDTCONFIG0_REG
    // system WDT
    /*
    write_addr_32((uint32_t *)0x3FF5F064, 0x050D83AA1); // TIMG0_T0_WDTWPROTECT_REG
    write_addr_32((uint32_t *)0x3FF60064, 0x050D83AA1); // TIMG1_T1_WDTWPROTECT_REG
    write_addr_32((uint32_t *)0x3FF5F048, 0x00);        // TIMG0_T0_WDTCONFIG0_REG
    write_addr_32((uint32_t *)0x3FF60048, 0x00);        // TIMG1_T1_WDTCONFIG0_REG
    */

    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {}

void arch::startup::stage3_startup() {
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::startup::kthread0() {}
