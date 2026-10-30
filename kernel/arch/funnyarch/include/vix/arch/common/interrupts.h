#pragma once
#include <vix/types.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 0;
    constexpr unsigned int INTERRUPT_STATE_ENABLED = 1;

    static inline unsigned int get_interrupt_state() {
        uint32_t pcst;
        asm volatile("mfsr %0, pcst" : "=r"(pcst));
        return (pcst >> 3) & 1;
    }

    static inline void set_interrupt_state(unsigned int state) {
        uint32_t pcst;
        asm volatile("mfsr %0, pcst" : "=r"(pcst));
        pcst &= ~(1 << 3);
        pcst |= (state & 1) << 3;
        asm volatile("mtsr pcst, %0" :: "r"(pcst));
    }
}
