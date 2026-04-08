#include <vix/arch/cpubasics.h>
#include <vix/initfn.h>
#include <vix/kprintf.h>
#include <vix/drivers/pci.h>

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

uint8_t x86_pci_config_read_8(struct pci::hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
    return pciConfigRead08(bus, device, function, offset);
}

uint16_t x86_pci_config_read_16(struct pci::hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
    return pciConfigRead16(bus, device, function, offset);
}

uint32_t x86_pci_config_read_32(struct pci::hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
    return pciConfigRead32(bus, device, function, offset);
}

void x86_pci_config_write_8(struct pci::hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint8_t value) {
    pciConfigWrite08(bus, device, function, offset, value);
}

void x86_pci_config_write_16(struct pci::hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint16_t value) {
    pciConfigWrite16(bus, device, function, offset, value);
}

void x86_pci_config_write_32(struct pci::hostbridge *hb, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint32_t value) {
    pciConfigWrite32(bus, device, function, offset, value);
}

void x86_pci_init() {
    static struct pci::hostbridge hb {
        .segment_group = 0,
        .pci_config_read_8 = x86_pci_config_read_8,
        .pci_config_read_16 = x86_pci_config_read_16,
        .pci_config_read_32 = x86_pci_config_read_32,
        .pci_config_write_8 = x86_pci_config_write_8,
        .pci_config_write_16 = x86_pci_config_write_16,
        .pci_config_write_32 = x86_pci_config_write_32,
    };
    pci::register_hostbridge(&hb);
}

INITFN_DEFINE(x86_pci, INITFN_EARLY_DRIVER_INIT, 0, x86_pci_init);
