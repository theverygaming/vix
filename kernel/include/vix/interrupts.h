#pragma once
#include <vix/arch/common/interrupts.h>
#include <vix/macros.h>
#include <vix/panic.h>
#include <vix/sched.h>

// NOTE: This is designed this way (context stored on active process) to support yielding from inside a push_interrupt_disable block
// FIXME: though to be quite honest I think this really should be revised.... Can we do without this bullshit??

inline void push_interrupt_disable() {
    if (unlikely(sched::mythread() == nullptr)) {
        return;
    }
    unsigned int state = arch::get_interrupt_state();
    arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED);
    if (sched::mythread()->pushpop_interrupt_count == 0) {
        sched::mythread()->pushpop_interrupt_state = state;
    }
    sched::mythread()->pushpop_interrupt_count++;
}

inline void pop_interrupt_disable() {
    if (unlikely(sched::mythread() == nullptr)) {
        return;
    }
    // TODO: maybe use assert here?
    // This should never happen, so we can disable it for non-debug builds
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED); // disable interrupts so panic can run safely
        KERNEL_PANIC("interrupts were not disabled on pop_interrupt_disable");
    }
    // TODO: maybe use assert here?
    if (sched::mythread()->pushpop_interrupt_count == 0) {
        KERNEL_PANIC("pop_interrupt_disable: popped too often!");
    }
    sched::mythread()->pushpop_interrupt_count--;
    if (sched::mythread()->pushpop_interrupt_count == 0) {
        arch::set_interrupt_state(sched::mythread()->pushpop_interrupt_state);
    }
}
