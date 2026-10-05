#pragma once
#include <vix/types.h>
#include <vix/arch/monitor.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 0;

    static inline unsigned int get_interrupt_state() {
        unsigned long flags = monitor_get_flags();
        return flags & MONITOR_FLAG_TIMER;
    }

    static inline void set_interrupt_state(unsigned int state) {
        unsigned long flags = monitor_get_flags();
        if ((state & MONITOR_FLAG_TIMER) != 0) {
            flags |= state & MONITOR_FLAG_TIMER;
        } else {
            flags &= ~MONITOR_FLAG_TIMER;
        }
        monitor_set_flags(flags);
    }
}
