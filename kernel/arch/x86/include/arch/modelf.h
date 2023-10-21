#pragma once
#include <arch/isr.h>
#include <string>
#include <types.h>
#include <vector>

namespace elf {
    void load_module(void *ELF_baseadr);
}
