#pragma once
#include <arch/isr.h>
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <types.h>

namespace elf {
    void load_module(void *ELF_baseadr);
}
