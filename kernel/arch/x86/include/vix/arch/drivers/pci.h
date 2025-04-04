#pragma once
#include <vix/types.h>

namespace drivers::pci {
    bool hasDev(uint16_t vendorID, uint16_t deviceID, uint8_t *bus, uint8_t *device, uint8_t *function);
    bool enableMastering(uint8_t bus, uint8_t device, uint8_t function);
    enum class bar_type { BAR_TYPE_ERROR, BAR_TYPE_MEMORY, BAR_TYPE_IO };
    bar_type getBarType(uint8_t bus, uint8_t device, uint8_t function, uint8_t barnum);
    uint16_t getBarIOAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t barnum);
    uint8_t getInterruptLine(uint8_t bus, uint8_t device, uint8_t function);
}
