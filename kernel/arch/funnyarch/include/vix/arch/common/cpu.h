#pragma once
#include <vix/types.h>

namespace arch {
    struct __attribute__((packed)) full_ctx {};

    struct __attribute__((packed)) ctx {
        uint32_t lr;
        uint32_t rfp;
        uint32_t r26;
        uint32_t r25;
        uint32_t r24;
        uint32_t r23;
        uint32_t r22;
        uint32_t r21;
        uint32_t r20;
        uint32_t r19;
        uint32_t r18;
        uint32_t r17;
        uint32_t r16;
        uint32_t r15;
        uint32_t r14;
        uint32_t r13;
        uint32_t r12;
        uint32_t r11;
        uint32_t r10;
        uint32_t r9;
        uint32_t r8;
    };
}
