#pragma once
#include <arch/generic/cpu.h>
#include <config.h>
#include <kprintf.h>
#include <macros.h>
#include <stdio.h>

namespace sched {
    void disable();
}

#define KERNEL_PANIC(reason)                                                                                                                   \
    do {                                                                                                                                       \
        sched::disable();                                                                                                                      \
        kprintf(KP_EMERG, "kernel panic in " __FILE__ " at line " TOSTRING(__LINE__) "\n    -> " reason "\n    -> %s\n", __PRETTY_FUNCTION__); \
        while (true) {                                                                                                                         \
            arch::generic::cpu::halt();                                                                                                        \
        }                                                                                                                                      \
    } while (0)
