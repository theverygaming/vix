#pragma once
#include <types.h>

namespace arch {
    struct __attribute__((packed)) full_ctx {};

    struct __attribute__((packed)) ctx {
        uint64_t x19;
        uint64_t x20;
        uint64_t x21;
        uint64_t x22;
        uint64_t x23;
        uint64_t x24;
        uint64_t x25;
        uint64_t x26;
        uint64_t x27;
        uint64_t x28;
        uint64_t fp;
        uint64_t lr;
    };
}
