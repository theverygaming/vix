#include <arch/generic/devices.h>
#include <time.h>

volatile uint64_t time::bootupTime = 0;

static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint64_t time::convertUTCToUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    uint64_t unixtime = 0;

    uint64_t unixyears = year - 1970;
    unixtime += ((unixyears + 2) / 4) * 86400; // leap years
    unixtime += unixyears * 86400 * 365;

    if (((year % 100) || (((year % 400) == 0))) && ((year % 4) == 0) && (month < 3)) { // is the current year a leap year and are we not in march yet?
        unixtime -= 86400;
    }

    month--;
    while (month) {
        month--;
        unixtime += monthDays[month] * 86400;
    }

    unixtime += (uint64_t)second;
    unixtime += ((uint64_t)minute) * 60;
    unixtime += ((uint64_t)hour) * 3600;
    unixtime += ((uint64_t)day - 1) * 86400;

    return unixtime;
}

uint64_t time::getCurrentUnixTime() {
    uint16_t year;
    uint8_t month, day, hour, minute, second;
    arch::generic::devices::get_current_rtc_time(&year, &month, &day, &hour, &minute, &second);
    return convertUTCToUnixTime(year, month, day, hour, minute, second);
}

uint64_t time::getUptimeSeconds() {
    return getCurrentUnixTime() - bootupTime;
}
