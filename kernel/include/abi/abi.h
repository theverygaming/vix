#pragma once
#include <config.h>

namespace abi {
    enum class type {
#ifdef CONFIG_ENABLE_ABI_LINUX
        LINUX,
#endif
#ifdef CONFIG_ENABLE_ABI_VIX
        VIX,
#endif
    };
}
