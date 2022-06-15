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
    gdt::i686_GDT_Initialize();
    clrscr();
    paging::initpaging();
    printf("Searching for hard drives\n"); 
    hdd::generic::scanDrives();
    if(hdd::generic::alldrives[0].alive) {
        printf("Trying to load data from HDD...\n");
        hdd::generic::readDrive((void*)KERNEL_VIRT_ADDRESS, 21, 56, hdd::generic::alldrives[0]);
        printf("Jumping to kernel\n");
        void (*kernel)(void) = (void (*)())KERNEL_VIRT_ADDRESS;
        kernel();
    }
    else {
        printf("no IDE hdd found. go get one\nSystem halted\n");
        asm("cli");
        asm("hlt");
    }
}