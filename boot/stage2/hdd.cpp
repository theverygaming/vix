#include "hdd.h"

#define PIO_BUF 0x1001000


hdd::ata_pio::atadevice_t cmd_identify(enum hdd::ata_pio::controller controller, enum hdd::ata_pio::drive drive) {
    hdd::ata_pio::atadevice_t device;
    device.dev_okay = false;
    uint16_t io_base = controller; // 0x1F0 controller 1, 0x170 controller 2
    uint16_t drv = drive; // 0xA0 master, 0xB0 slave
    outb(io_base + 0x06 , drv); // HDD selection
    outb(io_base + 0x02, 0);
    outb(io_base + 0x03, 0);
    outb(io_base + 0x04, 0);
    outb(io_base + 0x05, 0);

    outb(io_base + 0x07, 0xEC);
    if(inb(0x1F7)) {
        printf("Drive detected!\n");
        uint32_t i = 0;
        while(inb(io_base + 0x07) & (1 << 7)) { i++; if(i > 1000000) { printf("Loop stuck!\n"); break; } } //this loop is kinda bad, could possibly get stuck?
        if(inb(io_base + 0x04) || inb(io_base + 0x05)) {
            printf("Drive not ATA!\n");
            return device;
        }
        while(1) {
            char status = inb(io_base + 0x07);
            if(status & (1 << 0)) {
                printf("Error!\n");
                return device;
            }
            if(status & (1 << 3)) {
                printf("Ayo we fine and can read all data\n");
                uint16_t* buffer = (uint16_t*)PIO_BUF;
                for(int i = 0; i < 256; i++) {
                    buffer[i] = inw(io_base);
                }
                // The fuck IDE?
                for(int i = 0; i < 40; i += 2)
		        {
			        device.name[i] = ((char*)(PIO_BUF + 54))[i + 1];
                    device.name[i + 1] = ((char*)(PIO_BUF + 54))[i];
		        }
                device.name[41] = 0;
                printf("Disk label: ");
                printf(device.name);
                printf("\n");
                device.dev_okay = true;
                return device;
            }
        }
    }
    
}

void ide_400ns_delay(uint16_t io)
{
	for(int i = 0;i < 4; i++)
		inb(io + 0x0C);
}


void ide_poll(uint16_t io)
{
	
	for(int i=0; i< 4; i++)
		inb(io + 0x0C);

retry:;
	uint8_t status = inb(io + 0x07);
	if(status & 0x80) goto retry;
retry2:	status = inb(io + 0x07);
	if(status & 0x01)
	{
		printf("ERR set, device failure!\n");
	}
	if(!(status & 0x08)) goto retry2;
	return;
}

void ata_read_one_sector(void* buf, uint32_t lba, enum hdd::ata_pio::controller controller, enum hdd::ata_pio::drive drive) {
	/* We only support 28bit LBA so far */

	outb(controller + 0x06, ((drive + 0x40) | (uint8_t)((lba >> 24 & 0x0F))));
	outb(controller + 0x01, 0x00);
	outb(controller + 0x02, 1);
	outb(controller + 0x03, (uint8_t)((lba))); //LBA0
	outb(controller + 0x04, (uint8_t)((lba) >> 8)); //LBA1
	outb(controller + 0x05, (uint8_t)((lba) >> 16)); //LBA2
	outb(controller + 0x07, 0x20);

	ide_poll(controller);
	for(int i = 0; i < 256; i++)
	{
		uint16_t data = inw(controller + 0x00);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide_400ns_delay(controller);
}

void ata_read(void* buf, uint32_t lba, uint8_t sectors, enum hdd::ata_pio::controller controller, enum hdd::ata_pio::drive drive) {
    for(uint8_t i = 0; i < sectors; i++) {
        ata_read_one_sector(buf + (i * 512), lba + i, controller, drive);
    }
}


#include "../../config.h"

void hdd::ata_pio::idk() {
    printf("Searching for ATA drives\n"); 
    if(cmd_identify(ATA_CONTROLLER_PRIMARY, ATA_DRIVE_MASTER).dev_okay) { 
        printf("-> Primary master OK\n");
    }
    if(cmd_identify(ATA_CONTROLLER_PRIMARY, ATA_DRIVE_SLAVE).dev_okay) { 
        printf("-> Primary slave OK\n");
    }
    if(cmd_identify(ATA_CONTROLLER_SECONDARY, ATA_DRIVE_MASTER).dev_okay) { 
        printf("-> Secondary master OK\n");
    }
    if(cmd_identify(ATA_CONTROLLER_SECONDARY, ATA_DRIVE_SLAVE).dev_okay) { 
        printf("-> Secondary slave OK\n");
    }
    if(cmd_identify(ATA_CONTROLLER_PRIMARY, ATA_DRIVE_MASTER).dev_okay) {
        printf("Trying to load data from HDD...\n");
        ata_read((void*)KERNEL_VIRT_ADDRESS, 21, 56, ATA_CONTROLLER_PRIMARY, ATA_DRIVE_MASTER);
        printf("we survived, somehow\n");
        printf("Moment of truth: Jumping to kernel\n");
        void (*kernel)(void) = (void (*)())KERNEL_VIRT_ADDRESS;
        kernel();
    }
    else {
        printf("no IDE hdd found. go get one\nSystem halted\n");
        asm("cli");
        asm("hlt");
    }
}