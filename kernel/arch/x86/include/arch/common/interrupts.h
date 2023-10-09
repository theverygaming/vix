#pragma once
#include <config.h>
#include <types.h>

namespace arch {
    constexpr unsigned int INTERRUPT_STATE_DISABLED = 0;

    static inline unsigned int get_interrupt_state() {
#ifdef CONFIG_ENABLE_KERNEL_32
        uint32_t ef;
        asm volatile("pushfl; popl %0" : "=r"(ef));
#endif
#ifdef CONFIG_ENABLE_KERNEL_64
        uint64_t ef;
        asm volatile("pushfq; popq %0" : "=r"(ef));
#endif
        return ef & (1 << 9);
    }

    static inline void set_interrupt_state(unsigned int state) {
        if (state == 0) {
            asm volatile("cli");
        } else {
            asm volatile("sti");
        }
    }
}
