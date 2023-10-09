#pragma once
#include <types.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 0b1111;

    static inline unsigned int get_interrupt_state() {
        uint64_t daif;
        asm volatile("mrs %0, DAIF" : "=r"(daif));
        return (daif >> 6) & 0b1111;
    }

    static inline void set_interrupt_state(unsigned int state) {
        uint64_t daif;
        asm volatile("mrs %0, DAIF" : "=r"(daif));
        daif = (daif & ~(0b1111 << 6)) | ((state & 0b1111) << 6);
        asm volatile("msr DAIF, %0" : : "r"(daif));
    }
}
