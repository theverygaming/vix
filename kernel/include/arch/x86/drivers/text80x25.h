#pragma once
#include <types.h>

namespace drivers::textmode::text80x25 {
    void init();
    void putc(char c);
}
