#pragma once
#include <arch/generic/cpu.h>
#include <config.h>
#include <macros.h>
#include <stdio.h>
#include <tests/testprint.h>

#ifdef CONFIG_ENABLE_TESTS
#define KERNEL_PANIC(reason)                                                                             \
    do {                                                                                                 \
        TEST("kernel", "do not panic", false);                                                           \
        puts("kernel panic in " __FILE__ " at line " TOSTRING(__LINE__) "\n    -> " reason "\n    -> "); \
        puts(__PRETTY_FUNCTION__);                                                                       \
        puts("\n");                                                                                      \
        while (true) {                                                                                   \
            arch::generic::cpu::halt();                                                                  \
        }                                                                                                \
    } while (0)
#else
#define KERNEL_PANIC(reason)                                                                             \
    do {                                                                                                 \
        puts("kernel panic in " __FILE__ " at line " TOSTRING(__LINE__) "\n    -> " reason "\n    -> "); \
        puts(__PRETTY_FUNCTION__);                                                                       \
        while (true) {                                                                                   \
            arch::generic::cpu::halt();                                                                  \
        }                                                                                                \
    } while (0)
#endif
