#pragma once
#include <vix/config.h>
#include <vix/macros.h>
#include <vix/stdio.h>
#include <vix/types.h>

struct _test_def {
    bool (*testfn)();
    char name[40];
};

#ifdef CONFIG_ENABLE_TESTS
#define TEST_SYMBOL(name) __test_def_##name
#define TEST_STRUCT_SYMBOL(name) __test_struct_##name
#define TEST_FN_SYMBOL(name) __test_func_##name
#define TEST(_name)                                                        \
    static bool TEST_FN_SYMBOL(_name)();                                     \
    static struct _test_def TEST_STRUCT_SYMBOL(_name) = {                                             \
        .testfn = &TEST_FN_SYMBOL(_name),                                     \
        .name = STRINGIFY(_name),                                           \
    };                                               \
    extern "C" {                                                          \
    struct _test_def __attribute__((section(".test_defs"), used)) *TEST_SYMBOL(_name) = &TEST_STRUCT_SYMBOL(_name); \
    }                                                                     \
    static bool TEST_FN_SYMBOL(_name)()
#else
#define TEST(_name) \
    static inline bool __attribute__((always_inline)) TEST_FN_SYMBOL(_name)()
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
