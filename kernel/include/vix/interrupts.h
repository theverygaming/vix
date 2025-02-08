#pragma once
#include <vix/arch/common/interrupts.h>
#include <vix/macros.h>
#include <vix/panic.h>
#include <vix/sched.h>

inline void push_interrupt_disable() {
    if (unlikely(sched::mytask() == nullptr)) {
        return;
    }
    unsigned int state = arch::get_interrupt_state();
    arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED);
    if (sched::mytask()->pushpop_interrupt_count == 0) {
        sched::mytask()->pushpop_interrupt_state = state;
    }
    sched::mytask()->pushpop_interrupt_count++;
}

inline void pop_interrupt_disable() {
    if (unlikely(sched::mytask() == nullptr)) {
        return;
    }
    // TODO: maybe use assert here?
    // This should never happen, so we can disable it for non-debug builds
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED); // disable interrupts so panic can run safely
        KERNEL_PANIC("interrupts were not disabled on popcli");
    }
    // TODO: maybe use assert here?
    if (sched::mytask()->pushpop_interrupt_count == 0) {
        KERNEL_PANIC("popcli: popped too often!");
    }
    sched::mytask()->pushpop_interrupt_count--;
    if (sched::mytask()->pushpop_interrupt_count == 0) {
        arch::set_interrupt_state(sched::mytask()->pushpop_interrupt_state);
    }
}
