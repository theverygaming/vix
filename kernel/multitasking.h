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

    typedef struct {
        uint32_t pid;
        context registerContext;
        uint8_t priority;
        bool run;
    } process;
    void killCurrentProcess();
    void interruptTrigger();
    void create_task(void* stackadr, void* codeadr);
    process* getCurrentProcess();
    process* fork_process(process* process);
}
