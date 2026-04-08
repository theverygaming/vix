#include "vix/kernel/io.h"
#include <vix/kprintf.h>
#include <vix/panic.h>
#include <vix/drivers/pci.h>
#include <forward_list>

// FIXME: different layout? keep devices per HB?
static std::forward_list<struct pci::hostbridge *> hostbridges;
static std::forward_list<struct pci::pci_driver *> drivers;
static std::forward_list<struct pci::pci_dev *> devices_available;
struct device_adopted {
    struct pci::pci_dev *dev;
    struct pci::pci_driver *drv;
};
static std::forward_list<struct device_adopted> devices_adopted;

/* pci header structs */
struct pci_header_generic {
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
};

struct pci_header_0x0 {
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
};

/* pci header functions */
static struct pci_header_generic read_header_generic(struct pci::hostbridge *hb, uint8_t bus, uint8_t slot, uint8_t function) {
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
    struct pci_header_generic pciheader;
    pciheader.vendorID = hb->pci_config_read_16(hb, bus, slot, function, 0);
    pciheader.deviceID = hb->pci_config_read_16(hb, bus, slot, function, 2);
    pciheader.command = hb->pci_config_read_16(hb, bus, slot, function, 4);
    pciheader.status = hb->pci_config_read_16(hb, bus, slot, function, 6);
    pciheader.revisionID = hb->pci_config_read_8(hb, bus, slot, function, 8);
    pciheader.progIF = hb->pci_config_read_8(hb, bus, slot, function, 9);
    pciheader.subclass = hb->pci_config_read_8(hb, bus, slot, function, 10);
    pciheader.classID = hb->pci_config_read_8(hb, bus, slot, function, 11);
    pciheader.cacheLineSize = hb->pci_config_read_8(hb, bus, slot, function, 12);
    pciheader.latencyTimer = hb->pci_config_read_8(hb, bus, slot, function, 13);
    pciheader.headerType = hb->pci_config_read_8(hb, bus, slot, function, 14);
    pciheader.BIST = hb->pci_config_read_8(hb, bus, slot, function, 15);
    return pciheader;
}

static struct pci_header_0x0 read_header_0x0(struct pci::hostbridge *hb, uint8_t bus, uint8_t slot, uint8_t function) {
    struct pci_header_0x0 header;
    header.BAR0 = hb->pci_config_read_32(hb, bus, slot, function, 16);
    header.BAR1 = hb->pci_config_read_32(hb, bus, slot, function, 20);
    header.BAR2 = hb->pci_config_read_32(hb, bus, slot, function, 24);
    header.BAR3 = hb->pci_config_read_32(hb, bus, slot, function, 28);
    header.BAR4 = hb->pci_config_read_32(hb, bus, slot, function, 32);
    header.BAR5 = hb->pci_config_read_32(hb, bus, slot, function, 36);
    header.cardbusCISptr = hb->pci_config_read_32(hb, bus, slot, function, 40);
    header.subsystemVendorID = hb->pci_config_read_16(hb, bus, slot, function, 4);
    header.subsystemID = hb->pci_config_read_16(hb, bus, slot, function, 4);
    header.expansionROMbaseAddress = hb->pci_config_read_8(hb, bus, slot, function, 48);
    header.capabilitiesPtr = hb->pci_config_read_8(hb, bus, slot, function, 52);
    header.interruptLine = hb->pci_config_read_8(hb, bus, slot, function, 60);
    header.interruptPin = hb->pci_config_read_8(hb, bus, slot, function, 61);
    header.minGrant = hb->pci_config_read_8(hb, bus, slot, function, 62);
    header.maxLatency = hb->pci_config_read_8(hb, bus, slot, function, 63);
    return header;
}

/* might get rid of this depending on how useful it is */
static bool isMultiFunction(struct pci_header_generic header) {
    return (bool)(header.headerType & (1 << 7));
}

/* header type */
enum class PCIHeaderType { HEADER_TYPE_0x0, HEADER_TYPE_PCI_PCI_BRIDGE, HEADER_TYPE_PCI_CARDBUS_BRIDGE, HEADER_TYPE_NONE };
static PCIHeaderType get_header_type(struct pci_header_generic header) {
    switch (header.headerType & 0b01111111) {
    case 0x0:
        return PCIHeaderType::HEADER_TYPE_0x0;

    case 0x1:
        return PCIHeaderType::HEADER_TYPE_PCI_PCI_BRIDGE;

    case 0x2:
        return PCIHeaderType::HEADER_TYPE_PCI_CARDBUS_BRIDGE;

    default:
        return PCIHeaderType::HEADER_TYPE_NONE;
    }
}

static void enumerate_hostbridge(struct pci::hostbridge *hb) {
    for (unsigned int bus = 0; bus < 256; bus++) {
        for (unsigned int device = 0; device < 32; device++) {
            uint8_t function = 0;
            struct pci_header_generic pciHeader = read_header_generic(hb, bus, device, function);
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
                if (get_header_type(pciHeader) != PCIHeaderType::HEADER_TYPE_0x0) {
                    kprintf(KP_INFO, "PCI: ^ header not 0x0\n");
                }
                devices_available.push_front(new pci::pci_dev {
                    .hb = hb,
                    .bus = bus,
                    .device = device,
                    .function = function,
                });
                if (isMultiFunction(pciHeader)) {
                    for (function = 1; function < 8; function++) {
                        struct pci_header_generic pciHeader = read_header_generic(hb, bus, device, function);
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
                            if (get_header_type(pciHeader) != PCIHeaderType::HEADER_TYPE_0x0) {
                                kprintf(KP_INFO, "PCI: ^ header not 0x0\n");
                            }
                            devices_available.push_front(new pci::pci_dev {
                                .hb = hb,
                                .bus = bus,
                                .device = device,
                                .function = function,
                            });
                        }
                    }
                }
            }
        }
    }
}

static void try_attach_driver(struct pci::pci_driver *drv) {
    auto it = devices_available.begin();
    while (it != devices_available.end()) {
        struct pci::pci_dev *dev = *it;
        it++;
        // FIXME: reading the vendor ID like this each time is slow AF
        if (drv->vendor_id != pci::pci_dev_config_read_16(dev, 0) || drv->device_id != pci::pci_dev_config_read_16(dev, 2) || !drv->probe(dev)) {
            continue;
        }
        devices_available.erase_first_eq(dev);
        devices_adopted.push_front({
            .dev = dev,
            .drv = drv,
        });
        kprintf(KP_INFO, "pci: driver %s adopted device %p:%p.%p\n", drv->name, (uint32_t)dev->bus, (uint32_t)dev->device, (uint32_t)dev->function);
    }
}

static void attach_drivers() {
    for (auto it = drivers.begin(); it != drivers.end(); it++) {
        try_attach_driver(*it);
    }
}

namespace pci {
    void register_hostbridge(struct pci::hostbridge *hb) {
        hostbridges.push_front(hb);

        enumerate_hostbridge(hb);
        attach_drivers();
    }

    void register_driver(struct pci_driver *drv) {
        drivers.push_front(drv);
        try_attach_driver(drv);
    }

    uint8_t pci_dev_config_read_8(struct pci_dev *dev, uint16_t offset) {
        return dev->hb->pci_config_read_8(dev->hb, dev->bus, dev->device, dev->function, offset);
    }

    uint16_t pci_dev_config_read_16(struct pci_dev *dev, uint16_t offset) {
        return dev->hb->pci_config_read_16(dev->hb, dev->bus, dev->device, dev->function, offset);
    }

    uint32_t pci_dev_config_read_32(struct pci_dev *dev, uint16_t offset) {
        return dev->hb->pci_config_read_32(dev->hb, dev->bus, dev->device, dev->function, offset);
    }

    void pci_dev_config_write_8(struct pci_dev *dev, uint16_t offset, uint8_t value) {
        dev->hb->pci_config_write_8(dev->hb, dev->bus, dev->device, dev->function, offset, value);
    }

    void pci_dev_config_write_16(struct pci_dev *dev, uint16_t offset, uint16_t value) {
        dev->hb->pci_config_write_16(dev->hb, dev->bus, dev->device, dev->function, offset, value);
    }

    void pci_dev_config_write_32(struct pci_dev *dev, uint16_t offset, uint32_t value) {
        dev->hb->pci_config_write_32(dev->hb, dev->bus, dev->device, dev->function, offset, value);
    }

    struct bar pci_dev_get_bar(struct pci_dev *dev, uint8_t n) {
        // disable I/O and memory access while reading BAR (osdev wiki recommends this)
        uint16_t cmd_orig = pci_dev_config_read_16(dev, 4);
        pci_dev_config_write_16(dev, 4, cmd_orig & 0xFFFC);

        uint16_t bar_offset = 16 + (n * 4);

        struct bar bar;

        uint32_t bar_value = pci_dev_config_read_32(dev, bar_offset);

        uint32_t info_mask;
        
        // PMIO?
        if (bar_value & 0x1) {
            info_mask = 0xFFFFFFFC;
            bar.address = bar_value & info_mask;
            bar.mmio = false;
        } else { // MMIO
            info_mask = 0xFFFFFFF0;
            bar.mmio = true;
            bar.mmio_prefetchable = (bar_value & (1 << 3)) != 0;
            bar.address = bar_value & info_mask;
            // 64-bit BAR?
            if (((bar_value >> 1) & 0x3) == 0x2) {
                if (sizeof(bar.address) < 8) {
                    KERNEL_PANIC("64-bit BAR on 32-bit system?? (we don't support that yet)");
                }
                bar.address |= (uint64_t)pci_dev_config_read_32(dev, bar_offset + 4) << 32;
            }
        }

        // get BAR size
        pci_dev_config_write_32(dev, bar_offset, 0xFFFFFFFF);
        bar.len = ~(pci_dev_config_read_32(dev, bar_offset) & info_mask) + 1;
        pci_dev_config_write_32(dev, bar_offset, bar_value);

        // restore original command value
        pci_dev_config_write_16(dev, 4, cmd_orig);

        return bar;
    }

    io_handle_t pci_bar_iomap(struct bar bar) {
        if (bar.mmio) {
            // TODO: respect prefetch? (need io.h improvement for that, it io.h doesn't know cache stuff)
            return io_mmio_map(bar.address, bar.len);
        } else {
            return io_pmio_map(bar.address, bar.len);
        }
    }

    void pci_dev_master(struct pci_dev *dev, bool enable) {
        uint16_t command = pci_dev_config_read_16(dev, 4);
        if (enable) {
            command |= 1 << 2;
        } else {
            command &= ~(1 << 2);
        }
        pci_dev_config_write_16(dev, 4, command);
    }

    unsigned int pci_dev_get_irqline(struct pci_dev *dev) {
        return pci_dev_config_read_8(dev, 0x3C);
    }
}
