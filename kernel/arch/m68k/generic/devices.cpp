#include <arch/generic/devices.h>

void arch::generic::devices::get_current_rtc_time(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second) {
    *year = 1970;
    *month = 1;
    *day = 1;
    *hour = 0;
    *minute = 0;
    *second = 0;
}
