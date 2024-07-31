#pragma once
#include <vix/config.h>
#include <vix/stdio.h>

#ifdef CONFIG_ENABLE_TESTS
#define TEST(name) extern "C" bool __attribute__((section(".tests"))) __test_func_##name()
#else
#define TEST(name) static inline bool __attribute__((always_inline)) __test_func_##name()
#endif

#define TEST_ASSERT(condition)                               \
    do {                                                     \
        if (!(condition)) {                                  \
            printf("assertion \"" #condition "\" failed\n"); \
            return false;                                    \
        }                                                    \
    } while (0)

namespace test {
    void test_section(const char *name, bool status);
}
