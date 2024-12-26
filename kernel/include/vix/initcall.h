#pragma once
#include <vix/macros.h>

typedef int (*initcall_t)();

void initarr_init_level(int level);

#define _DEFINE_INITCALL(level, function) static initcall_t __initcall_##function __attribute__((section(".initcall" TOSTRING(level)), used)) = function

#define INITCALL(function) _DEFINE_INITCALL(0, function)
