#pragma once
#include <vix/macros.h>

typedef int (*initcall_t)();

void initcall_init_level(int level);

#define _DEFINE_INITCALL(level, function) \
    static initcall_t __initcall_##function __attribute__((section(".initcall" TOSTRING(level)), used)) = function

// level 0: called shortly after memory allocators are initialized
#define INITCALL_0(function) _DEFINE_INITCALL(0, function)

// level 1: called from inside the very first scheduler thread (thread 0)
#define INITCALL_1(function) _DEFINE_INITCALL(1, function)
