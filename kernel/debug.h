#pragma once
#include "stdio.h"
#include "types.h"

#define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)
#define DEBUG_BREAKPOINT debug::breakpoint(__LINE__, __FILE__)
#define DEBUG_VISUAL(character) debug::visual_dbg(character)

namespace debug {
    void stack_trace(uint32_t maxLength, uint32_t ebp);
    void debug_loop();
    void breakpoint(int line, char *file);
    void visual_dbg(char c);
}