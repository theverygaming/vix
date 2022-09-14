#pragma once

#include <generated/config.h>
#define ARCH_EXPAND(x) x

#define ARCH_CONCAT_INCLUDE(a, b) ARCH_EXPAND(<arch/)ARCH_EXPAND(a)ARCH_EXPAND(/)ARCH_EXPAND(b)ARCH_EXPAND(>)

#define INCLUDE_ARCH(i) ARCH_CONCAT_INCLUDE(CONFIG_ARCH, i)
