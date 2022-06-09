#include "hdd.h"

void cmd_identify() {
    outb(0x1F6, 0xA0); // 0xA0 master, 0xB0 slave
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    outb(0x1F7, 0xEC);
    if(inb(0x1F7)) {
        printf("Drive detected!\n");
    }
    else {
        printf("no drive detected\n");
    }
    
}

void hdd::idk() {
    printf("Searching for ATA drives\n"); 
    cmd_identify();
    printf("survided\n");
}