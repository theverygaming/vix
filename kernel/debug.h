#pragma once
#include "types.h"
#include <panic.h>
#include <stdio.h>

#define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)
#define DEBUG_BREAKPOINT debug::breakpoint(__LINE__, __FILE__)
#define DEBUG_VISUAL(character) debug::visual_dbg(character)
#define assertm(condition, msg) do { if(!(condition)) { KERNEL_PANIC("assertion failed - " msg); } } while(0)

namespace debug {
    void stack_trace(uint32_t maxLength, uint32_t ebp);
    void debug_loop();
    void breakpoint(int line, char *file);
    void visual_dbg(char c);
}