#pragma once

#include <vix/arch/cpubasics.h>
#include <vix/types.h>

namespace drivers::serial {
    void init();
    void putc(char c);
    char getc();
}
