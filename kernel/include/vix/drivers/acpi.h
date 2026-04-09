#pragma once
#include <vix/config.h>

#ifdef CONFIG_ENABLE_ACPI
namespace acpi {
    void reboot();
    void shutdown();
}
#endif
