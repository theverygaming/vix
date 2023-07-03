#pragma once
#include <config.h>

#ifdef CONFIG_ENABLE_TESTS
#define TEST(name) extern "C" bool __attribute__((section(".tests"))) __test_func_##name()
#else
#define TEST(name) static inline bool __attribute__((always_inline)) __test_func_##name()
#endif

namespace test {
    void test_section(const char *name, bool status);
}
