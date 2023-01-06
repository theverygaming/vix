#pragma once
#include <types.h>

namespace drivers::keyboard {
    void init();
    void poll();
    extern char buffer[100];
    extern int bufferlocation;
}

namespace drivers::mouse {
    void init();
}
