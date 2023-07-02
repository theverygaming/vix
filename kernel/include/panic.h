#pragma once
#include <arch/generic/cpu.h>
#include <config.h>
#include <macros.h>
#include <stdio.h>

#define KERNEL_PANIC(reason)                                                                             \
    do {                                                                                                 \
        puts("kernel panic in " __FILE__ " at line " TOSTRING(__LINE__) "\n    -> " reason "\n    -> "); \
        puts(__PRETTY_FUNCTION__);                                                                       \
        while (true) {                                                                                   \
            arch::generic::cpu::halt();                                                                  \
        }                                                                                                \
    } while (0)
