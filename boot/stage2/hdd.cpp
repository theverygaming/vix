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
        uint32_t i = 0;
        while(inb(io_base + 0x07) & (1 << 7)) { i++; if(i > 1000000) { break; } } //this loop is kinda bad, could possibly get stuck?
        if(inb(io_base + 0x04) || inb(io_base + 0x05)) {
            return device;
        }
        while(1) {
            char status = inb(io_base + 0x07);
            if(status & (1 << 0)) {
                return device;
            }
            if(status & (1 << 3)) {
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
                device.dev_okay = true;
                return device;
            }
        }
    }
    
}

static void ide_400ns_delay(uint16_t io)
{
	for(int i = 0;i < 4; i++)
		inb(io + 0x0C);
}

static void ide_poll(uint16_t io)
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

static void ata_read_one_sector(void* buf, uint32_t lba, enum hdd::ata_pio::controller controller, enum hdd::ata_pio::drive drive) {
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

static void ata_read(void* buf, uint32_t lba, uint16_t sectors, enum hdd::ata_pio::controller controller, enum hdd::ata_pio::drive drive) {
    for(uint16_t i = 0; i < sectors; i++) {
        ata_read_one_sector(buf + (i * 512), lba + i, controller, drive);
    }
}

int hdd::ata_pio::generic_get_drives(int baseindex, int maxsize, hdd::generic::genericDrive_t* drivearr) {
    hdd::generic::genericDrive_t drives[4] = {false, hdd::generic::genericDrive_t::GENERIC_DRIVE_ATAPIO, 0};
    int counter = 0;

    if(cmd_identify(ATA_CONTROLLER_PRIMARY, ATA_DRIVE_MASTER).dev_okay) {
        drives[0].alive = true;
        drives[0].devinfo1 = ATA_CONTROLLER_PRIMARY;
        drives[0].devinfo2 = ATA_DRIVE_MASTER;
        printf("ATA PIO -> Primary master found(%s)\n", cmd_identify(ATA_CONTROLLER_PRIMARY, ATA_DRIVE_MASTER).name);
        counter++;
    }
    if(cmd_identify(ATA_CONTROLLER_PRIMARY, ATA_DRIVE_SLAVE).dev_okay) { 
        drives[1].alive = true;
        drives[1].devinfo1 = ATA_CONTROLLER_PRIMARY;
        drives[1].devinfo2 = ATA_DRIVE_SLAVE;
        printf("ATA PIO -> Primary slave found(%s)\n", cmd_identify(ATA_CONTROLLER_PRIMARY, ATA_DRIVE_SLAVE).name);
        counter++;
    }
    if(cmd_identify(ATA_CONTROLLER_SECONDARY, ATA_DRIVE_MASTER).dev_okay) { 
        drives[2].alive = true;
        drives[2].devinfo1 = ATA_CONTROLLER_SECONDARY;
        drives[2].devinfo2 = ATA_DRIVE_MASTER;
        printf("ATA PIO -> Secondary master found(%s)\n", cmd_identify(ATA_CONTROLLER_SECONDARY, ATA_DRIVE_MASTER).name);
        counter++;
    }
    if(cmd_identify(ATA_CONTROLLER_SECONDARY, ATA_DRIVE_SLAVE).dev_okay) { 
        drives[3].alive = true;
        drives[3].devinfo1 = ATA_CONTROLLER_SECONDARY;
        drives[3].devinfo2 = ATA_DRIVE_SLAVE;
        printf("ATA PIO -> Secondary slave found(%s)\n", cmd_identify(ATA_CONTROLLER_SECONDARY, ATA_DRIVE_SLAVE).name);
        counter++;
    }

    if(baseindex + counter >= maxsize) { return counter * -1; };
    int indx = 0;
    for(int i = 0; i < 4; i++) {
        if(drives[i].alive) {
            indx++;
            drivearr[baseindex + indx] = drives[i];
        } 
    }
    return baseindex + indx;
}

void hdd::ata_pio::generic_read(void* buf, uint64_t lba, uint16_t sectors, hdd::generic::genericDrive_t drive) {
    enum hdd::ata_pio::controller controller = (hdd::ata_pio::controller)drive.devinfo1;
    enum hdd::ata_pio::drive drv = (hdd::ata_pio::drive)drive.devinfo2;
    ata_read(buf, lba, sectors, controller, drv);
}

#include "../../config.h"
void hdd::ata_pio::idk() {
    printf("Searching for ATA drives\n"); 
    int ab = generic_get_drives(-1, 4, (hdd::generic::genericDrive_t*)PIO_BUF);
    printf("ab: %d\n", ab);
    if(((hdd::generic::genericDrive_t*)PIO_BUF)[0].alive) {
        printf("Trying to load data from HDD...\n");
        generic_read((void*)KERNEL_VIRT_ADDRESS, 21, 56, ((hdd::generic::genericDrive_t*)PIO_BUF)[0]);
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
    return;
}