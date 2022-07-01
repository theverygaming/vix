#pragma once
#include "paging.h"
#include "types.h"
#include "../config.h"

#define PROCESS_MAX_PAGE_RANGES 20

namespace multitasking
{
    typedef struct {
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
        uint32_t phys_base;
        uint32_t virt_base;
        uint32_t pages;
    } process_pagerange;

    typedef struct {
        uint32_t pid;
        context registerContext;
        uint8_t priority;
        bool running;
        process_pagerange pages[PROCESS_MAX_PAGE_RANGES];
    } process;

    void killCurrentProcess();
    void interruptTrigger();
    void create_task(void* stackadr, void* codeadr);
    process* getCurrentProcess();
    process* fork_process(process* process);
    void setProcessSwitching(bool state);
    bool createPageRange(process_pagerange* range, uint32_t max_address = KERNEL_VIRT_ADDRESS);
    void setPageRange(process_pagerange* range);
    void unsetPageRange(process_pagerange* range);
}
