#include "../config.h"
#include "elf.h"
#include "memorymap.h"
#include "multitasking.h"
#include "stdio.h"
#include "stdlib.h"
#include "syscall.h"
#include <arch/arch.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/paging.h>
#include <memory_alloc/memalloc.h>
#include INCLUDE_ARCH(cpuid.h)
#include INCLUDE_ARCH(simd.h)
#include INCLUDE_ARCH(cpubasics.h)
#include "cpp.h"
#include <arch/x86/drivers/keyboard.h>
#include <arch/x86/drivers/pci.h>
#include <arch/x86/drivers/serial.h>
#include <panic.h>
#include <multiboot2.h>

void kernelstart(void *multiboot2_info_ptr);

extern "C" void __attribute__((section(".entry"))) _start(void *multiboot2_info_ptr) {
    size_t sp;
    asm volatile("mov %%esp, %0" : "=a"(sp) :);
    if (sp < KERNEL_VIRT_ADDRESS) {
        return;
    }
    kernelstart(multiboot2_info_ptr);
    while (true) {}
}

// very important array definitely
uint8_t franxxlogo[9][18] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4, 0},
    {0, 0, 0, 0, 0, 0, 0, 15, 0, 15, 0, 0, 0, 4, 0, 4, 0, 0},
    {0, 0, 1, 0, 1, 0, 1, 0, 15, 0, 4, 0, 4, 0, 4, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0},
    {0, 0, 1, 0, 1, 0, 1, 0, 15, 0, 4, 0, 4, 0, 4, 0, 0, 0},
    {0, 1, 0, 1, 0, 0, 0, 15, 0, 15, 0, 0, 0, 4, 0, 4, 0, 0},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

void kernelstart(void *multiboot2_info_ptr) {
    for (uint32_t i = 0; i < 0xFFFFFF; i++) {}
    clrscr();
    drivers::serial::init();
    printf("hewwo\n");
    printf("shitOS built %s %s\n", __DATE__, __TIME__);
    if ((size_t)multiboot2_info_ptr & 7) {
        KERNEL_PANIC("multiboot2 info structure is not aligned, something is wrong here");
    }
    int memMap_count = 0;
    void *memMap = multiboot2::findMemMap(multiboot2_info_ptr, &memMap_count);
    memorymap::initMemoryMap(memMap, memMap_count);
    paging::clearPageTables((void *)0x0, KERNEL_VIRT_ADDRESS / 4096);
    memalloc::page::phys_init(memorymap::map_entries, memorymap::map_entrycount);
    memalloc::page::kernel_init();
    memalloc::page::kernel_alloc((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET), 704);
    cpubasics::cpuinit();
    cpp_init();
    multitasking::initMultitasking();
    drivers::keyboard::init();
    isr::RegisterHandler(0x80, syscall::syscallHandler);
    cpuid::printFeatures();
    simd::enableSSE();
    elf::load_program((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
    memalloc::page::kernel_free((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
    // drivers::pci::init();
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 18; j++) {
            putcolor(j + 63, i, franxxlogo[i][j] << 4 | 7);
        }
    }

    multitasking::process_pagerange prange[PROCESS_MAX_PAGE_RANGES];
    multitasking::createPageRange(prange);
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        if (prange[i].pages != 0) {
            printf("created prange, pb: 0x%p vb: 0x%p pg: %u, i: %d\n", prange[i].phys_base, prange[i].virt_base, prange[i].pages, i);
        }
    }

    int counter = 0;
    while (true) {
        *((unsigned volatile char *)((KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET) + 2 * 70 + 160 * 0)) = counter / 20;
        counter++;
        if (counter == 1000) {
            counter = 0;
        }
    }
}