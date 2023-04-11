#pragma once
#include <arch/isr.h>
#include <string>
#include <vector>
#include <types.h>

namespace elf {
    void load_module(void *ELF_baseadr);
}
