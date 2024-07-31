#pragma once
#include <vix/types.h>

namespace arch {
    struct __attribute__((packed)) full_ctx {};

    struct __attribute__((packed)) ctx {
        uint32_t ra;
        // FIXME
        /*uint32_t a2;
        uint32_t a3;
        uint32_t a4;
        uint32_t a5;
        uint32_t a6;
        uint32_t d2;
        uint32_t d3;
        uint32_t d4;
        uint32_t d5;
        uint32_t d6;
        uint32_t d7;
        uint32_t pc;*/
    };
}
