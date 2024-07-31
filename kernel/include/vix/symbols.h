#pragma once
#include <utility>
#include <vix/types.h>

namespace syms {
    // returns null on failure
    uintptr_t get_sym(const char *name);

    // increment n until nullptr is returned in first
    std::pair<const char *, uintptr_t> get_sym(size_t n);

    // first is nullptr on error
    std::pair<const char *, uintptr_t> find_func_sym(uintptr_t addr);
}
