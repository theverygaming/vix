#pragma once
#include "../types.h"

namespace drivers::keyboard {
    void init();
    extern char buffer[100];
    extern int bufferlocation;
}