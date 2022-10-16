#pragma once

#define KERNEL_PANIC(reason) panic::kernel_panic(__FILE__, __LINE__, reason)

namespace panic {
    void kernel_panic(const char *file, int line, const char *text);
}
