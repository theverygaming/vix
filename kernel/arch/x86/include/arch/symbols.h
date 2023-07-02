#pragma once
#include <types.h>
#include <utility>

namespace syms {
    // returns null on failure
    uintptr_t get_sym(const char *name);

    // increment n until nullptr is returned in first
    std::pair<const char *, uintptr_t> get_sym(size_t n);
}
