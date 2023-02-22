#include <arch/generic/devices.h>
#include <time.h>

volatile int64_t time::bootupTime = 0;
volatile uint64_t time::ms_since_bootup = 0;

static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int64_t time::convertUTCToUnixTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    int64_t unixtime = 0;

    int64_t unixyears = year - 1970;
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

    unixtime += (int64_t)second;
    unixtime += ((int64_t)minute) * 60;
    unixtime += ((int64_t)hour) * 3600;
    unixtime += ((int64_t)day - 1) * 86400;

    return unixtime;
}

int64_t time::getCurrentUnixTime() {
    uint16_t year;
    uint8_t month, day, hour, minute, second;
    arch::generic::devices::get_current_rtc_time(&year, &month, &day, &hour, &minute, &second);
    return convertUTCToUnixTime(year, month, day, hour, minute, second);
}

int64_t time::getUptimeSeconds() {
    return getCurrentUnixTime() - bootupTime;
}
