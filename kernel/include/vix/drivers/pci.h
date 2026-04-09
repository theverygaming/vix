#pragma once
#include <vix/kernel/io.h>
#include <vix/types.h>

namespace pci {
    struct hostbridge {
        uint16_t segment_group;
        uint8_t (*pci_config_read_8)(struct hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset);
        uint16_t (*pci_config_read_16)(struct hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset);
        uint32_t (*pci_config_read_32)(struct hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset);
        void (*pci_config_write_8)(struct hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint8_t value);
        void (*pci_config_write_16)(struct hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint16_t value);
        void (*pci_config_write_32)(struct hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint32_t value);
    };

    struct bar {
        uintptr_t address;
        size_t len;
        bool mmio;
        bool mmio_prefetchable;
    };

    struct pci_dev {
        struct hostbridge *hb;
        uint8_t bus;
        uint8_t device;
        uint8_t function;
    };

    struct pci_driver {
        const char *name;
        // TODO: id table
        uint16_t vendor_id;
        uint16_t device_id;
        bool (*probe)(struct pci_dev *dev); /* returns true when the driver has adopted the device */
        //void (*remove)(struct pci_dev *dev); // FIXME:
    };

    void register_hostbridge(struct hostbridge *hb);
    void register_driver(struct pci_driver *drv);

    struct pci_dev *pci_dev_open_force(uint16_t segment_group, uint8_t bus, uint8_t device, uint8_t function) ;
    void pci_dev_close(struct pci_dev *dev);

    uint8_t pci_dev_config_read_8(struct pci_dev *dev, uint16_t offset);
    uint16_t pci_dev_config_read_16(struct pci_dev *dev, uint16_t offset);
    uint32_t pci_dev_config_read_32(struct pci_dev *dev, uint16_t offset);
    void pci_dev_config_write_8(struct pci_dev *dev, uint16_t offset, uint8_t value);
    void pci_dev_config_write_16(struct pci_dev *dev, uint16_t offset, uint16_t value);
    void pci_dev_config_write_32(struct pci_dev *dev, uint16_t offset, uint32_t value);
    struct bar pci_dev_get_bar(struct pci_dev *dev, uint8_t n);
    io_handle_t pci_bar_iomap(struct bar);
    void pci_dev_master(struct pci_dev *dev, bool enable);
    unsigned int pci_dev_get_irqline(struct pci_dev *dev);
}
