#pragma once

namespace mm::kv {
    void init();
    void *alloc(size_t pages);
    void free(void *address, size_t pages);
}
