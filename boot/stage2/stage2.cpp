void stage2start(void);

void _start(void)
{
    stage2start();
}

#include "gdt.h"
#include "paging.h"
#include "stdio.h"
#include "hdd.h"
#include "../../config.h"

void stage2start(void) {
    clrscr();
    paging::initpaging();
    gdt::i686_GDT_Initialize();
    printf("Searching for hard drives\n");
    hdd::generic::scanDrives();
    if(hdd::generic::alldrives[0].alive) {
        printf("Trying to load data from HDD...\n");
        hdd::generic::readDrive((void*)KERNEL_VIRT_ADDRESS, 21, 200, hdd::generic::alldrives[0]);
        hdd::generic::readDrive((void*)(KERNEL_VIRT_ADDRESS + KERNEL_FREE_AREA_BEGIN_OFFSET), 2048, 1954, hdd::generic::alldrives[0]);
        printf("Jumping to kernel\n");
        uint32_t sp_adr = KERNEL_VIRT_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET;
        uint32_t kerneladr = KERNEL_VIRT_ADDRESS;
        asm volatile("" : : "a"(sp_adr));
        asm("mov %eax, %esp");
        asm volatile("" : : "a"(kerneladr));
        asm("jmpl %eax");
    }
    else {
        printf("no IDE hdd found. go get one\nSystem halted\n");
        asm("cli");
        asm("hlt");
    }
}