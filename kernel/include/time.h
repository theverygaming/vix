#pragma once
#include <types.h>

namespace time {
    extern volatile uint64_t bootupTime;
    uint64_t convertUTCToUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    uint64_t getCurrentUnixTime();
    uint64_t getUptimeSeconds();
}
