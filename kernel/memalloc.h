#pragma once
#include "types.h"

namespace memalloc::page {
    void* malloc(uint32_t blocks);
    void free(void* adr);
}