#pragma once

#include <arch/cpubasics.h>
#include <types.h>

namespace drivers::serial {
    void init();
    void putc(char c);
    char getc();
}
