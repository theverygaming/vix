#include "hdd.h"

#define PIO_BUF 0x1001000


hdd::atadevice_t cmd_identify() {
    hdd::atadevice_t device;
    device.dev_okay = false;
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
            return device;
        }
        while(1) {
            char status = inb(0x1F7);
            if(status & (1 << 0)) {
                printf("Error!\n");
                return device;
            }
            if(status & (1 << 3)) {
                printf("Ayo we fine and can read all data\n");
                uint16_t* buffer = (uint16_t*)PIO_BUF;
                for(int i = 0; i < 256; i++) {
                    buffer[i] = inw(0x1F0);
                }
                // The fuck IDE?
                for(int i = 0; i < 40; i += 2)
		        {
			        device.name[i] = ((char*)(PIO_BUF + 54))[i + 1];
                    device.name[i + 1] = ((char*)(PIO_BUF + 54))[i];
		        }
                device.name[41] = 0;
                printf(device.name);
                device.dev_okay = true;
                return device;
            }
        }
    }
    
}

void hdd::idk() {
    printf("Searching for ATA drives\n"); 
    cmd_identify();
    printf("survided\n");
}