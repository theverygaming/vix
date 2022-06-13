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

void ata_read_one_sector(void* buf, uint32_t lba) {
	/* We only support 28bit LBA so far */
	uint8_t drive = 0x00; // Master drive
	uint16_t io = 0x1F0; // Primary IO

	uint8_t cmd = (drive==0x00?0xE0:0xF0);
	uint8_t slavebit = (drive == 0x00?0x00:0x01);
	outb(io + 0x06, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	outb(io + 1, 0x00);
	outb(io + 0x02, 1);
	outb(io + 0x03, (uint8_t)((lba))); //LBA0
	outb(io + 0x04, (uint8_t)((lba) >> 8)); //LBA1
	outb(io + 0x05, (uint8_t)((lba) >> 16)); //LBA2
	outb(io + 0x07, 0x20);

	ide_poll(io);
	for(int i = 0; i < 256; i++)
	{
		uint16_t data = inw(io + 0x00);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide_400ns_delay(io);
}

void ata_read(void* buf, uint32_t lba, uint8_t sectors) {
    for(uint8_t i = 0; i < sectors; i++) {
        ata_read_one_sector(buf + (i * 512), lba + i);
    }
}


#include "../../config.h"

void hdd::idk() {
    printf("Searching for ATA drives\n"); 
    if(cmd_identify().dev_okay) {
        printf("Trying to load data from HDD...\n");
        ata_read((void*)KERNEL_VIRT_ADDRESS, 21, 56);
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