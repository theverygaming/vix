#pragma once
#include <types.h>

namespace time {
    uint64_t convertToUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute);
}
