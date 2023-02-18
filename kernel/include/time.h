#pragma once
#include <types.h>

namespace time {
    extern volatile int64_t bootupTime;
    int64_t convertUTCToUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    int64_t getCurrentUnixTime();
    int64_t getUptimeSeconds();
}
