#pragma once
#include <vix/macros.h>

void __attribute__((noreturn)) kernel_panic(const char *panic_fmt, ...);

#define KERNEL_PANIC(reason, ...)                                                                                                               \
    do {                                                                                                                                        \
        kernel_panic(                                                                                                                           \
            "panic in " __FILE__ " at line " STRINGIFY(__LINE__) " in function %s -- reason: " reason "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__); \
    } while (0)
