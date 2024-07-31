#pragma once
#include <vix/arch/common/interrupts.h>
#include <vix/panic.h>

extern unsigned int pushpop_interrupt_state;
extern unsigned int pushpop_interrupt_count;

inline void push_interrupt_disable() {
    unsigned int state = arch::get_interrupt_state();
    arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED);
    if (pushpop_interrupt_count == 0) {
        pushpop_interrupt_state = state;
    }
    pushpop_interrupt_count++;
}

inline void pop_interrupt_disable() {
    // TODO: maybe use assert here?
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED); // disable interrupts so panic can run safely
        KERNEL_PANIC("interrupts were not disabled on popcli");
    }
    pushpop_interrupt_count--;
    if (pushpop_interrupt_count == 0) {
        arch::set_interrupt_state(pushpop_interrupt_state);
    }
}
