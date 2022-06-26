#include "debug.h"
#include "stdio.h"
#include "paging.h"

void debug::stack_trace(uint32_t maxLength, uint32_t ebp) {
    printf("Stack trace:\n");
    for(uint32_t i = 0; ebp && i < maxLength; i++)
    {
        if(!paging::is_readable((void*)ebp) && !paging::is_readable((void*)ebp + 4)) { break; }
        uint32_t eip = *((uint32_t*)ebp + 4);
        printf("->0x%p\n", eip);
        ebp = *((uint32_t*)ebp);
    }
}