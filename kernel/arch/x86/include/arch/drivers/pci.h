#pragma once
#include <types.h>

namespace drivers::pci {
    uint32_t pciConfigRead32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
    uint16_t pciConfigRead16(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
    uint8_t pciConfigRead08(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
    void pciConfigWrite32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t data);
    void pciConfigWrite16(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint16_t data);
    void pciConfigWrite08(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint8_t data);
    void init();
    bool hasDev(uint16_t vendorID, uint16_t deviceID, uint8_t *bus, uint8_t *device, uint8_t *function);
    bool enableMastering(uint8_t bus, uint8_t device, uint8_t function);
    enum class bar_type { BAR_TYPE_ERROR, BAR_TYPE_MEMORY, BAR_TYPE_IO };
    bar_type getBarType(uint8_t bus, uint8_t device, uint8_t function, uint8_t barnum);
    uint16_t getBarIOAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t barnum);
    uint8_t getInterruptLine(uint8_t bus, uint8_t device, uint8_t function);
}
