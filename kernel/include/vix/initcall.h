#pragma once
#include <vix/macros.h>

typedef int (*initcall_t)();

void initcall_init_level(int level);

#define DEFINE_INITCALL(level, priority, function) \
    static initcall_t __initcall_##function __attribute__((section(".initcall" TOSTRING(level) "." TOSTRING(priority)), used)) = function

#define INITCALL_PRIO_DEFAULT 1024

// called almost immediately after control is handed to the kernel, shortly prior to initializing the allocators
#define INITCALL_PRE_MM_INIT 0
// called shortly after memory allocators have been initialized and C++ constructors called but before the scheduler has been initialized
#define INITCALL_EARLY_DRIVER_INIT 1
// called from inside the very first scheduler thread (thread 0)
#define INITCALL_DRIVER_INIT 2
