#include "pci.h"
#include "../cpubasics.h"
#include "../stdio.h"

static uint16_t pciConfigRead(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) | (((uint32_t)func) << 8) | (offset & 0xFC) | ((uint32_t)0x80000000);
    outl(0xCF8, address);
    return (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

static uint16_t checkDevice(uint8_t bus, uint8_t slot) {

    // get vendor ID
    uint16_t vendor, device;
    if ((vendor = pciConfigRead(bus, slot, 0, 0)) != 0xFFFF) {
        device = pciConfigRead(bus, slot, 0, 2);
        /* */
        printf("PCI - %p:%p -> vendor: 0x%p device: 0x%p\n", (uint32_t)bus, (uint32_t)slot, (uint32_t)vendor, (uint32_t)device);
    }
    return vendor;
}

void drivers::pci::init() {
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            checkDevice(bus, device);
        }
    }
}