#include <arch/x86/archinit.h>
#include <arch/x86/drivers/serial.h>
#include <arch/x86/drivers/text80x25.h>
#include <arch/x86/multiboot2.h>
#include <arch/x86/paging.h>
#include <config.h>
#include <kernel.h>
#include <memorymap.h>
#include <panic.h>
#include <stdio.h>

static void kernelinit(void *multiboot2_info_ptr) {
    for (uint32_t i = 0; i < 0xFFFFFF; i++) {}
    drivers::textmode::text80x25::init();
    stdio::set_putc_function(drivers::textmode::text80x25::putc);
    drivers::serial::init();
    stdio::set_putc_function(drivers::serial::putc, true);
    if ((size_t)multiboot2_info_ptr & 7) {
        KERNEL_PANIC("multiboot2 info structure is not aligned, something is wrong here");
    }
    int memMap_count = 0;
    void *memMap = multiboot2::findMemMap(multiboot2_info_ptr, &memMap_count);
    memorymap::initMemoryMap(memMap, memMap_count);
    paging::clearPageTables((void *)0x0, KERNEL_VIRT_ADDRESS / 4096);
    kernelstart();
}

extern "C" void __attribute__((section(".entry"))) _kentry(void *multiboot2_info_ptr) {
    size_t sp;
    asm volatile("mov %%esp, %0" : "=a"(sp) :);
    if (sp < KERNEL_VIRT_ADDRESS) {
        return;
    }
    kernelinit(multiboot2_info_ptr);
    while (true) {}
}
