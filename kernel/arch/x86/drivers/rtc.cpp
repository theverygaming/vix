#include <string.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/drivers/rtc.h>

#define CMOS_PORT 0x70
#define CMOS_DATA 0x71

static inline uint8_t getCMOSregister(uint8_t reg) {
    outb(CMOS_PORT, reg);
    return inb(CMOS_DATA);
}

static inline bool isCMOSupdateInProgress() {
    return (getCMOSregister(0x0A) & 0x80) > 0;
}

static struct drivers::rtc::time getCMOSTime() {
    struct drivers::rtc::time currentTime;
    while (isCMOSupdateInProgress()) {}
    uint8_t regB = getCMOSregister(0x0B);

    currentTime.second = getCMOSregister(0x00);
    currentTime.minute = getCMOSregister(0x02);
    currentTime.hour = getCMOSregister(0x04);
    currentTime.day = getCMOSregister(0x07);
    currentTime.month = getCMOSregister(0x08);
    currentTime.year = getCMOSregister(0x09);
    if (isCMOSupdateInProgress()) {
        return getCMOSTime();
    }
    if (!(regB & 0x04)) {
        currentTime.second = (currentTime.second & 0x0F) + ((currentTime.second / 16) * 10);
        currentTime.minute = (currentTime.minute & 0x0F) + ((currentTime.minute / 16) * 10);
        currentTime.hour = ((currentTime.hour & 0x0F) + (((currentTime.hour & 0x70) / 16) * 10)) | (currentTime.hour & 0x80);
        currentTime.day = (currentTime.day & 0x0F) + ((currentTime.day / 16) * 10);
        currentTime.month = (currentTime.month & 0x0F) + ((currentTime.month / 16) * 10);
        currentTime.year = (currentTime.year & 0x0F) + ((currentTime.year / 16) * 10);
    }
    return currentTime;
}

namespace drivers::rtc {
    struct time getCurrentTime() {
        struct time lastTime;
        memset(&lastTime, 0, sizeof(struct time));
        struct time currentTime = getCMOSTime();
        while (memcmp(&currentTime, &lastTime, sizeof(struct time)) != 0) {
            lastTime = currentTime;
            currentTime = getCMOSTime();
        }
        return currentTime;
    }
}
