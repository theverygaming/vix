#pragma once
#include <arch/x86/paging.h>
#include <config.h>
#include <cppstd/vector.h>
#include <types.h>

#define PROCESS_MAX_PAGE_RANGES 20
#define MAX_PROCESSES 100

namespace multitasking {
    typedef struct {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
        uint32_t esi;
        uint32_t edi;
        uint32_t esp;
        uint32_t ebp;

        uint16_t ds;
        uint16_t es;
        uint16_t fs;
        uint16_t gs;
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
        volatile bool running;
        enum class privilege {KERNEL, USER} privilege;
        process_pagerange pages[PROCESS_MAX_PAGE_RANGES];
    } process;

    void initMultitasking();
    bool isProcessSwitchingEnabled();
    void killCurrentProcess();
    void interruptTrigger();
    void create_task(void *stackadr, void *codeadr, process_pagerange *pagerange, std::vector<char *> *argv);
    void replace_task(void *stackadr, void *codeadr, process_pagerange *pagerange, std::vector<char *> *argv, int replacePid);
    process *getCurrentProcess();
    void waitForProcess(int pid);
    void refresh_current_process_pagerange();
    process *fork_current_process();
    void setProcessSwitching(bool state);
    bool createPageRange(process_pagerange *range, uint32_t max_address = KERNEL_VIRT_ADDRESS);
    void setPageRange(process_pagerange *range);
    void unsetPageRange(process_pagerange *range);
    void zeroPageRange(process_pagerange *range);
    void freePageRange(process_pagerange *range);
    void printPageRange(process_pagerange *range);
}
