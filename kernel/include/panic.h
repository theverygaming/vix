#pragma once
#include <arch/generic/cpu.h>
#include <config.h>
#include <macro.h>
#include <stdio.h>
#include <tests/testprint.h>

#ifdef CONFIG_ENABLE_TESTS
#define KERNEL_PANIC(reason)                                                                 \
    do {                                                                                     \
        TEST("kernel", "do not panic", false);                                               \
        puts("kernel panic in " __FILE__ " at line " TOSTRING(__LINE__) "\n    -> " reason); \
        while (true) {                                                                       \
            arch::generic::cpu::halt();                                                      \
        }                                                                                    \
    } while (0)
#else
#define KERNEL_PANIC(reason)                                                                 \
    do {                                                                                     \
        puts("kernel panic in " __FILE__ " at line " TOSTRING(__LINE__) "\n    -> " reason); \
        while (true) {                                                                       \
            arch::generic::cpu::halt();                                                      \
        }                                                                                    \
    } while (0)
#endif
