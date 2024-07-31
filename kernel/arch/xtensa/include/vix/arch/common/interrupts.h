#pragma once
#include <vix/types.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 15;

    static inline unsigned int get_interrupt_state() {
        uint32_t ps;
        asm volatile("rsr.ps %0" : "=a"(ps));
        return (ps & 0xF);
    }

    static inline void set_interrupt_state(unsigned int state) {
        uint32_t ps;
        asm volatile("rsr.ps %0" : "=a"(ps));
        ps = (ps & ~0xFul) | (state & 0xF);
        asm volatile("wsr.ps %0; rsync" : : "a"(ps));
    }
}
