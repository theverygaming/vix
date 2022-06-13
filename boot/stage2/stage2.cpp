void stage2start(void);

void _start(void)
{
    stage2start();
}

#include "gdt.h"
#include "paging.h"
#include "stdio.h"
#include "hdd.h"

void stage2start(void) {
    gdt::i686_GDT_Initialize();
    clrscr();
    paging::initpaging();
    printf("we are still alive!\n");
    //*((unsigned char *)(0xB8000 + 2 * 79 + 160 * 0)) = 'U';
    hdd::ata_pio::idk();
    while(1) {
        
    }
}