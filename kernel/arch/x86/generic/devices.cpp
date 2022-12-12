#include <arch/drivers/rtc.h>
#include <arch/generic/devices.h>

void arch::generic::devices::get_current_rtc_time(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second) {
    struct drivers::rtc::time currentTime = drivers::rtc::getCurrentTime();
    *year = currentTime.year + 2000; // this should work for a while...
    *month = currentTime.month;
    *day = currentTime.day;
    *hour = currentTime.hour;
    *minute = currentTime.minute;
    *second = currentTime.second;
}
