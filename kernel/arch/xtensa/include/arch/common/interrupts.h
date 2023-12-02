#pragma once
#include <types.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 15;

    static inline unsigned int get_interrupt_state() {
        unsigned int current;
        asm volatile("rsr.ps %0" : "=a"(current));
        return current;
    }

    static inline void set_interrupt_state(unsigned int state) {
        asm volatile("wsr.ps %0; rsync" : "=a"(state));
    }
}
