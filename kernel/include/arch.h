#pragma once

#include <generated/autoconf.h>

#define ARCH_EXPAND(x) x

#define ARCH_CONCAT_INCLUDE(a, b) ARCH_EXPAND(<arch/)ARCH_EXPAND(b)ARCH_EXPAND(>)

#define INCLUDE_ARCH_GENERIC(i) ARCH_CONCAT_INCLUDE(CONFIG_ARCH,generic/i)
