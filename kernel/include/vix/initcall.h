#pragma once
#include <vix/macros.h>

typedef int (*initcall_t)();

void initcall_init_level(int level);

#define DEFINE_INITCALL(level, function) \
    static initcall_t __initcall_##function __attribute__((section(".initcall" TOSTRING(level)), used)) = function

// called shortly after memory allocators are initialized
#define INITCALL_AFTER_MM_INIT 0
// called from inside the very first scheduler thread (thread 0)
#define INITCALL_FIRST_THREAD 1
