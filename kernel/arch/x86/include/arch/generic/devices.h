#pragma once
#include <types.h>

namespace arch::generic::devices {
    /*
     * This function must return the current time in UTC
     * if the architecture does not provide an RTC this function must still return a valid date
     */
    void get_current_rtc_time(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second);
}
