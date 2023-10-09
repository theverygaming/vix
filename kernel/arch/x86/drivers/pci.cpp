#include <arch/cpubasics.h>
#include <arch/drivers/pci.h>
#include <kprintf.h>

/* direct read/write functions */
static inline uint32_t pciConfigRead32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t address = (((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) | (((uint32_t)function) << 8) | (offset & 0xFC) | (0x80000000);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static inline uint16_t pciConfigRead16(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return (uint16_t)((pciConfigRead32(bus, slot, function, offset) >> ((offset % 4) * 8)) & 0xFFFF);
}

static inline uint8_t pciConfigRead08(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return (uint8_t)((pciConfigRead32(bus, slot, function, offset) >> ((offset % 4) * 8)) & 0xFF);
}

static inline void pciConfigWrite32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data) {
    uint32_t address = (((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) | (((uint32_t)function) << 8) | (offset & 0xFC) | (0x80000000);
    outl(0xCF8, address);
    outl(0xCFC, data);
}

static inline void pciConfigWrite16(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint16_t data) {
    uint32_t read = pciConfigRead32(bus, slot, function, offset);
    read &= ~(0xFFFF << ((offset % 4) * 8));
    read |= data << ((offset % 4) * 8);
    pciConfigWrite32(bus, slot, function, offset, read);
}

static inline void pciConfigWrite08(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t data) {
    uint32_t read = pciConfigRead32(bus, slot, function, offset);
    read &= ~(0xFF << ((offset % 4) * 8));
    read |= data << ((offset % 4) * 8);
    pciConfigWrite32(bus, slot, function, offset, read);
}

/* pci header structs */

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

typedef struct {
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t cardbusCISptr;
    uint16_t subsystemID;
    uint16_t subsystemVendorID;
    uint32_t expansionROMbaseAddress;
    uint8_t capabilitiesPtr;
    uint8_t maxLatency;
    uint8_t minGrant;
    uint8_t interruptPin;
    uint8_t interruptLine;
} pciHeader0x0_t;

/* pci header functions */
static generic_pciHeader_t readGenericHeader(uint8_t bus, uint8_t slot, uint8_t function) {
    /*
     * +----------+--------+----------+------------+--------------+---------------+
     * | Register | Offset | Byte3    | Byte2      | Byte1        | Byte0         |
     * +----------+--------+----------+------------+--------------+---------------+
     * | 0x0      | 0x0    |      deviceID         |           vendorID           |
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

static pciHeader0x0_t read0x0Header(uint8_t bus, uint8_t slot, uint8_t function) {
    pciHeader0x0_t header;
    header.BAR0 = pciConfigRead32(bus, slot, function, 16);
    header.BAR1 = pciConfigRead32(bus, slot, function, 20);
    header.BAR2 = pciConfigRead32(bus, slot, function, 24);
    header.BAR3 = pciConfigRead32(bus, slot, function, 28);
    header.BAR4 = pciConfigRead32(bus, slot, function, 32);
    header.BAR5 = pciConfigRead32(bus, slot, function, 36);
    header.cardbusCISptr = pciConfigRead32(bus, slot, function, 40);
    header.subsystemVendorID = pciConfigRead16(bus, slot, function, 44);
    header.subsystemID = pciConfigRead16(bus, slot, function, 46);
    header.expansionROMbaseAddress = pciConfigRead32(bus, slot, function, 48);
    header.capabilitiesPtr = pciConfigRead08(bus, slot, function, 52);
    header.interruptLine = pciConfigRead08(bus, slot, function, 60);
    header.interruptPin = pciConfigRead08(bus, slot, function, 61);
    header.minGrant = pciConfigRead08(bus, slot, function, 62);
    header.maxLatency = pciConfigRead08(bus, slot, function, 63);
    return header;
}

/* might get rid of this depending on how useful it is */
static bool isMultiFunction(generic_pciHeader_t header) {
    return (bool)(header.headerType & (1 << 7));
}

/* header type */
enum class pciHeaderType { HEADER_TYPE_0x0, HEADER_TYPE_PCI_PCI_BRIDGE, HEADER_TYPE_PCI_CARDBUS_BRIDGE, HEADER_TYPE_NONE };
static pciHeaderType getHeaderType(generic_pciHeader_t header) {
    switch (header.headerType & 0b01111111) {
    case 0x0:
        return pciHeaderType::HEADER_TYPE_0x0;

    case 0x1:
        return pciHeaderType::HEADER_TYPE_PCI_PCI_BRIDGE;

    case 0x2:
        return pciHeaderType::HEADER_TYPE_PCI_CARDBUS_BRIDGE;

    default:
        return pciHeaderType::HEADER_TYPE_NONE;
    }
}

void drivers::pci::init() {
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            uint8_t function = 0;
            generic_pciHeader_t pciHeader = readGenericHeader(bus, device, function);
            if (pciHeader.vendorID != 0xFFFF) {
                kprintf(KP_INFO,
                        "PCI: - %p:%p.%p -> type: 0x%p 0x%p vendor: 0x%p device: 0x%p\n",
                        (uint32_t)bus,
                        (uint32_t)device,
                        (uint32_t)function,
                        (uint32_t)pciHeader.classID,
                        (uint32_t)pciHeader.subclass,
                        (uint32_t)pciHeader.vendorID,
                        (uint32_t)pciHeader.deviceID);
                if (getHeaderType(pciHeader) != pciHeaderType::HEADER_TYPE_0x0) {
                    kprintf(KP_INFO, "PCI: ^ header not 0x0\n");
                }
                if (isMultiFunction(pciHeader)) {
                    for (function = 1; function < 8; function++) {
                        generic_pciHeader_t pciHeader = readGenericHeader(bus, device, function);
                        if (pciHeader.vendorID != 0xFFFF) {
                            kprintf(KP_INFO,
                                    "PCI: - %p:%p.%p -> type: 0x%p 0x%p vendor: 0x%p device: 0x%p\n",
                                    (uint32_t)bus,
                                    (uint32_t)device,
                                    (uint32_t)function,
                                    (uint32_t)pciHeader.classID,
                                    (uint32_t)pciHeader.subclass,
                                    (uint32_t)pciHeader.vendorID,
                                    (uint32_t)pciHeader.deviceID);
                            if (getHeaderType(pciHeader) != pciHeaderType::HEADER_TYPE_0x0) {
                                kprintf(KP_INFO, "PCI: ^ header not 0x0\n");
                            }
                        }
                    }
                }
            }
        }
    }
}

bool drivers::pci::hasDev(uint16_t vendorID, uint16_t deviceID, uint8_t *bus, uint8_t *device, uint8_t *function) {
    for (int _bus = 0; _bus < 256; _bus++) {
        for (int _device = 0; _device < 32; _device++) {
            uint8_t _function = 0;
            generic_pciHeader_t pciHeader = readGenericHeader(_bus, _device, _function);
            if (pciHeader.vendorID != 0xFFFF) {
                if ((pciHeader.vendorID == vendorID) && (pciHeader.deviceID == deviceID)) { // for now we only support devices with header 0x0
                    *bus = _bus;
                    *device = _device;
                    *function = _function;
                    return true;
                }
                if (isMultiFunction(pciHeader)) {
                    for (_function = 1; _function < 8; _function++) {
                        generic_pciHeader_t pciHeader = readGenericHeader(_bus, _device, _function);
                        if ((pciHeader.vendorID == vendorID) && (pciHeader.deviceID == deviceID)) { // for now we only support devices with header 0x0
                            *bus = _bus;
                            *device = _device;
                            *function = _function;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool drivers::pci::enableMastering(uint8_t bus, uint8_t device, uint8_t function) {
    generic_pciHeader_t pciHeader = readGenericHeader(bus, device, function);
    if ((pciHeader.command >> 2) & 0x1) { // already enabled?
        return true;
    }
    pciConfigWrite16(bus, device, function, 4, pciHeader.command | (0x1 << 2));
    pciHeader = readGenericHeader(bus, device, function);
    if ((pciHeader.command >> 2) & 0x1) { // did that work?
        return true;
    }
    return false;
}

drivers::pci::bar_type drivers::pci::getBarType(uint8_t bus, uint8_t device, uint8_t function, uint8_t barnum) { // TODO: check memory layout
    if (barnum > 5) {
        return bar_type::BAR_TYPE_ERROR;
    }
    uint32_t BAR = pciConfigRead32(bus, device, function, 16 + (barnum * 4));
    if (BAR & 0x1) {
        return bar_type::BAR_TYPE_IO;
    } else {
        return bar_type::BAR_TYPE_MEMORY;
    }
}

uint16_t drivers::pci::getBarIOAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t barnum) {
    if (barnum > 5 || getBarType(bus, device, function, barnum) != bar_type::BAR_TYPE_IO) {
        return 0;
    }
    uint32_t BAR = pciConfigRead32(bus, device, function, 16 + (barnum * 4));
    return (uint16_t)(BAR & 0xFFFFFFFC);
}

uint8_t drivers::pci::getInterruptLine(uint8_t bus, uint8_t device, uint8_t function) {
    return pciConfigRead08(bus, device, function, 60);
}
