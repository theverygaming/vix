#pragma once
#include <vix/arch/isr.h>
#include <string>
#include <vix/types.h>
#include <vector>

namespace elf {
    void load_module(void *ELF_baseadr);
}
