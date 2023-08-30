#pragma once
#include <types.h>

typedef int64_t time_t;

namespace time {
    extern volatile time_t bootupTime;
    extern volatile uint64_t ns_since_bootup; // used for precise timing, not gurateed to be available and incrementing
    time_t convertUTCToUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    time_t getCurrentUnixTime();
    time_t getUptimeSeconds();
}
