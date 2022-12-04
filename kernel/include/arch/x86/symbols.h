#pragma once
#include <types.h>

namespace syms {
    // returns null on failure
    uintptr_t get_sym(const char *name);
}
