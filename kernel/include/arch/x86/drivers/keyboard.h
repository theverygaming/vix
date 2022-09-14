#pragma once
#include <types.h>

namespace drivers::keyboard {
    void init();
    void manualRead();
    extern char buffer[100];
    extern int bufferlocation;
}
