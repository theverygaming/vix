#pragma once
#include <types.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 6;

    static inline unsigned int get_interrupt_state() {
        uint32_t sr;
        // FIXME:
        //asm volatile("move.w %%sr, %0" : "=r"(sr));
        return (sr >> 8) & 0b111;
    }

    static inline void set_interrupt_state(unsigned int state) {
        uint32_t sr;
        // FIXME:
        //asm volatile("move.w %%sr, %0" : "=r"(sr));
        sr = (sr & ~(0b111 << 8)) | ((state & 0b111) << 8);
        //asm volatile("move.w %0, %%sr" : : "r"(sr));
    }
}
