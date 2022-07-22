#pragma once

#include "../types.h"
#include "../cpubasics.h"


namespace drivers::serial {
    void init();
    void putc(char c);
    char getc();
}