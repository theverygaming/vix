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
        while(inb(0x1F7) & (1 << 7)) { } //this loop is kinda bad, could possibly get stuck?
        if(inb(0x1F4) || inb(0x1F5)) {
            printf("Drive not ATA!\n");
            return;
        }
        while(1) {
            char status = inb(0x1F7);
            if(status & (1 << 0)) {
                printf("Error!\n");
                return;
            }
            if(status & (1 << 3)) {
                printf("Ayo we fine and can read all data\n");
                uint16_t id0, id83, id88, id93, id60, id61, id100, id101, id103;
                for(int i = 0; i < 256; i++) {
                    uint16_t value = inw(0x1F0);
                    if(i == 0) {
                        id0 = value;
                        printf("id0: %d\n", id0);
                    }
                    if(i == 83) {
                        id83 = value;
                        printf("id83: %d\n", id83);
                        if(id83 & (1 << 10)) {
                            printf("LBA48 supported\n");
                        }
                    }
                    if(i == 88) {
                        id88 = value;
                        printf("id88: %d\n", id88);
                    }
                    if(i == 93) {
                        id93 = value;
                        printf("id93: %d\n", id93);
                    }
                    if(i == 60) {
                        id60 = value;
                        printf("id60: %d\n", id60);
                    }
                    if(i == 61) {
                        id61 = value;
                        printf("id61: %d\n", id61);
                    }
                }
                return;
            }
        }
    }
    
}

void hdd::idk() {
    printf("Searching for ATA drives\n"); 
    cmd_identify();
    printf("survided\n");
}