#pragma once
#include <string>
#include <vector>
#include <vix/arch/isr.h>
#include <vix/types.h>

namespace elf {
    void load_module(void *ELF_baseadr);
}
