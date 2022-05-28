#pragma once
#include "paging.h"
#include "types.h"

namespace multitasking
{
    typedef struct{
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
        uint32_t esi;
        uint32_t edi;
        uint32_t esp;
        uint32_t ebp;
    } context;
    
    void interruptTrigger();
}
