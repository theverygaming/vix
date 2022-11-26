#include <arch/lx106_esp8266/generic/memory.h>
#include <arch/lx106_esp8266/generic/startup.h>
#include <kernel.h>
#include <memory_alloc/memalloc.h>
#include <stdio.h>
#include <types.h>

int (*ets_uart_printf)(const char *fmt, ...) = (int (*)(const char *, ...))0x40002544;
static void putc(char c) {
    ets_uart_printf("%c", c);
}

static void kernelinit() {
    // stdio::set_putc_function(putc);
    stdio::set_putc_function(putc, true);
    kernelstart();
}

void (*uart_div_modify)(int uart, int divisor) = (void (*)(int, int))0x400039d8;
void (*rom_i2c_writeReg)(int reg, int hosid, int par, int val) = (void (*)(int, int, int, int))0x400072d8;

extern "C" uint32_t _bss_start;
extern "C" uint32_t _bss_end;

extern "C" void __attribute__((section(".entry"))) _kentry() {
    // 160MHz overclock, we wanna go speedy
    rom_i2c_writeReg(103, 4, 1, 0xc8);
    rom_i2c_writeReg(103, 4, 2, 0x91);

    for (uint32_t *addr = &_bss_start; addr < &_bss_end; addr++) {
        *addr = 0;
    }
    uart_div_modify(0, (160 * 1000000) / 115200);
    ets_uart_printf("\n\n");
    kernelinit();
    ets_uart_printf("kernelinit returned");
    while (true) {}
}

extern "C" uint32_t _rodata_start;
extern "C" uint32_t _rodata_end;
extern "C" uint32_t _data_start;
extern "C" uint32_t _data_end;

void arch::generic::startup::stage2_startup() {
    ets_uart_printf("free kernel memory: %u bytes\n", memalloc::page::kernel_get_free_blocks() * ARCH_PAGE_SIZE);
    ets_uart_printf("free physical memory: %u bytes\n", memalloc::page::phys_get_free_blocks() * ARCH_PAGE_SIZE);
    // allocate memory used by sections in kernel
    memalloc::page::kernel_alloc((void *)((size_t)&_bss_start - ARCH_PAGE_SIZE), ((((size_t)&_bss_end) - ((size_t)&_bss_start)) / ARCH_PAGE_SIZE) + 1);
    memalloc::page::kernel_alloc((void *)((size_t)&_rodata_start - ARCH_PAGE_SIZE), ((((size_t)&_rodata_end) - ((size_t)&_rodata_start)) / ARCH_PAGE_SIZE) + 1);
    memalloc::page::kernel_alloc((void *)((size_t)&_data_start - ARCH_PAGE_SIZE), ((((size_t)&_data_end) - ((size_t)&_data_start)) / ARCH_PAGE_SIZE) + 1);
    ets_uart_printf("free kernel memory: %u bytes\n", memalloc::page::kernel_get_free_blocks() * ARCH_PAGE_SIZE);
    ets_uart_printf("free physical memory: %u bytes\n", memalloc::page::phys_get_free_blocks() * ARCH_PAGE_SIZE);
    uint64_t *testp = (uint64_t *)memalloc::page::kernel_malloc(1);
    ets_uart_printf("malloc gave us memory %u, lets try using it...\n", testp);
    testp[0] = 8734872473;
    if (testp[0] == 8734872473) {
        ets_uart_printf("memory works!\n");
    }
    memalloc::page::kernel_free(testp);
    ets_uart_printf("we can use it!\n");
    ets_uart_printf("but what about single?\n");
    uint64_t *test = (uint64_t *)memalloc::single::kmalloc(8);
    memalloc::single::kfree(test);
    ets_uart_printf("fuck yeah!\n");
}
