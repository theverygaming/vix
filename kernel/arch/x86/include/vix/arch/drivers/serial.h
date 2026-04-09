#pragma once

#include <vix/arch/cpubasics.h>
#include <vix/types.h>

namespace drivers::serial {
    void init();
    void putb(uint8_t b);
    uint8_t getb();
    void putc(char c);
    void puts(const char *str, size_t n);
    char getc();
}
