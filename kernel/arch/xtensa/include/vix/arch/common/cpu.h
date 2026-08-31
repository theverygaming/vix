#pragma once
#include <vix/types.h>
#include <vix/config.h>

namespace arch {
    struct __attribute__((packed)) full_ctx {};

    struct __attribute__((packed)) ctx {
#ifdef CONFIG_XTENSA_TARGET_ESP8266
        uint32_t a0;
        uint32_t a12;
        uint32_t a13;
        uint32_t a14;
        uint32_t a15;
#endif
    };
}
