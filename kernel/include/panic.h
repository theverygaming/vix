#pragma once
#include <arch/generic/cpu.h>
#include <macro.h>
#include <stdio.h>

#define KERNEL_PANIC(reason)                                                                 \
    do {                                                                                     \
        puts("kernel panic in " __FILE__ " at line " TOSTRING(__LINE__) "\n    -> " reason); \
        while (true) {                                                                       \
            arch::generic::cpu::halt();                                                      \
        }                                                                                    \
    } while (0)
