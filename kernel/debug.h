#pragma once
#include "types.h"
#include "stdio.h"

#define DEBUG_PRINTF(...) printf_serial(__VA_ARGS__)

namespace debug {
    void stack_trace(uint32_t maxLength, uint32_t ebp);
}