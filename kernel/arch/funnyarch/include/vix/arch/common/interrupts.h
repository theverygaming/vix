#pragma once
#include <vix/types.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 7;

    static inline unsigned int get_interrupt_state() {
        // FIXME: to be implemented
        return 7;
    }

    static inline void set_interrupt_state(unsigned int state) {
        // FIXME: to be implemented
    }
}
