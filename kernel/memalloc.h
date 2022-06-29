#pragma once
#include "types.h"

namespace memalloc {
    void* malloc(uint32_t blocks);
    void free(void* adr);
}