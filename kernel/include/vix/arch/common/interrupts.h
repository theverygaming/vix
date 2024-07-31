#pragma once

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 0;

    static inline unsigned int get_interrupt_state() {
#error "get_interrupt_state unimplemented"
    }

    static inline void set_interrupt_state(unsigned int state) {
#error "set_interrupt_state unimplemented"
    }
}
