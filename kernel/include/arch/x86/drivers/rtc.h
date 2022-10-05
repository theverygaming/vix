#pragma once
#include <types.h>

namespace drivers::rtc {
    struct time {
        uint8_t second;
        uint8_t minute;
        uint8_t hour;
        uint8_t day;
        uint8_t month;
        uint8_t year;
        uint8_t century; // unimplemented
    };

    struct time getCurrentTime();

    uint64_t getunixtime();
}
