#pragma once
#include "types.h"
#include "stdio.h"

#define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)
#define DEBUG_BREAKPOINT debug::breakpoint(__LINE__, __FILE__)

namespace debug {
    void stack_trace(uint32_t maxLength, uint32_t ebp);
    void debug_loop();
    void breakpoint(int line, char* file);
}