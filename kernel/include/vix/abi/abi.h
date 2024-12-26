#pragma once
#include <vix/config.h>

namespace abi {
    enum class type {
        KERNEL_ONLY,
#ifdef CONFIG_ENABLE_ABI_LINUX
        LINUX,
#endif
#ifdef CONFIG_ENABLE_ABI_VIX
        VIX,
#endif
    };
}
