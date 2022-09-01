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
#include <fs/roramfs.h>
#include <fs/vfs.h>
#include <multiboot2.h>
#include <panic.h>

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
    printf("hewwo\n");
    printf("shitOS built %s %s\n", __DATE__, __TIME__);
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
    fs::filesystems::roramfs::init((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
    fs::filesystems::roramfs::mountInVFS();
    void *elfptr = nullptr;
    if (fs::vfs::fptr("/ramfs/shitshell", &elfptr)) {
        elf::load_program(elfptr);
    }
    // elf::load_program((void *)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET));
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