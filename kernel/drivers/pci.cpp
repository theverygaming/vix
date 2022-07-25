#include "pci.h"
#include "../cpubasics.h"
#include "../stdio.h"

/* direct read/write functions */
static uint32_t pciConfigRead32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t address = (((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) | (((uint32_t)function) << 8) | (offset & 0xFC) | ((uint32_t)0x80000000);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static uint16_t pciConfigRead16(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return (uint16_t)((pciConfigRead32(bus, slot, function, offset) >> ((offset & 2) * 8)) & 0xFFFF);
}

static uint8_t pciConfigRead08(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return (uint8_t)((pciConfigRead32(bus, slot, function, offset) >> (offset * 8)) & 0xFF);
}

/* pci header functions */
typedef struct {
    uint16_t deviceID;
    uint16_t vendorID;
    uint16_t status;
    uint16_t command;
    uint8_t classID;
    uint8_t subclass;
    uint8_t progIF;
    uint8_t revisionID;
    uint8_t BIST;
    uint8_t headerType;
    uint8_t latencyTimer;
    uint8_t cacheLineSize;
} generic_pciHeader_t;

static generic_pciHeader_t readGenericHeader(uint8_t bus, uint8_t slot, uint8_t function) {
    /*
     * +----------+--------+----------+------------+--------------+---------------+
     * | Register | Offset | Byte3    | Byte2      | Byte1        | Byte0         |
     * +----------+--------+----------+------------+--------------+---------------+
     * | 0x0      | 0x0    |      deviceID         |           vedorID            |
     * | 0x1      | 0x4    |       status          |           command            |
     * | 0x2      | 0x8    | classID  | subclass   | progIF       | revisionID    |
     * | 0x3      | 0xC    | BIST     | headerType | latencyTimer | cacheLineSize |
     * +----------+--------+----------+------------+--------------+---------------+
     */
    generic_pciHeader_t pciheader;
    pciheader.vendorID = pciConfigRead16(bus, slot, function, 0);
    pciheader.deviceID = pciConfigRead16(bus, slot, function, 2);
    pciheader.command = pciConfigRead16(bus, slot, function, 4);
    pciheader.status = pciConfigRead16(bus, slot, function, 6);
    pciheader.revisionID = pciConfigRead08(bus, slot, function, 8);
    pciheader.progIF = pciConfigRead08(bus, slot, function, 9);
    pciheader.subclass = pciConfigRead08(bus, slot, function, 10);
    pciheader.classID = pciConfigRead08(bus, slot, function, 11);
    pciheader.cacheLineSize = pciConfigRead08(bus, slot, function, 12);
    pciheader.latencyTimer = pciConfigRead08(bus, slot, function, 13);
    pciheader.headerType = pciConfigRead08(bus, slot, function, 14);
    pciheader.BIST = pciConfigRead08(bus, slot, function, 15);
    return pciheader;
}

/* might get rid of this depending on how useful it is */
static bool isMultiFunction(generic_pciHeader_t *header) {
    return (bool)(header->headerType & (1 << 7));
}

void drivers::pci::init() {
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            uint8_t function = 0;
            generic_pciHeader_t pciHeader = readGenericHeader(bus, device, function);
            if (pciHeader.vendorID != 0xFFFF) {
                printf("PCI - %p:%p.%p -> type: 0x%p 0x%p vendor: 0x%p device: 0x%p\n", (uint32_t)bus, (uint32_t)device, (uint32_t)function, (uint32_t)pciHeader.classID, (uint32_t)pciHeader.subclass, (uint32_t)pciHeader.vendorID, (uint32_t)pciHeader.deviceID);
                if (isMultiFunction(&pciHeader)) {
                    for (function = 1; function < 8; function++) {
                        generic_pciHeader_t pciHeader = readGenericHeader(bus, device, function);
                        if (pciHeader.vendorID != 0xFFFF) {
                            printf("PCI - %p:%p.%p -> type: 0x%p 0x%p vendor: 0x%p device: 0x%p\n", (uint32_t)bus, (uint32_t)device, (uint32_t)function, (uint32_t)pciHeader.classID, (uint32_t)pciHeader.subclass, (uint32_t)pciHeader.vendorID, (uint32_t)pciHeader.deviceID);
                        }
                    }
                }
            }
        }
    }
}