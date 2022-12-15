#pragma once
#include <arch/isr.h>
#include <arch/paging.h>
#include <config.h>
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <scheduler.h>
#include <types.h>

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

        uint16_t cs;
        uint32_t eip;
        uint32_t eflags; // can the user even touch these?
        uint16_t ss;
    } context;

    typedef struct {
        uint32_t phys_base;
        uint32_t virt_base;
        uint32_t pages;
        /*
         * static -> code ,bss etc.
         * break -> memory allocated by brk()
         */
        enum class range_type { UNKNOWN, STATIC, BREAK } type;
    } process_pagerange;

    class x86_process : public schedulers::generic_process {
    public:
        context registerContext;
        std::vector<process_pagerange> pages;
        uintptr_t brk_start = 0;
    };

    void initMultitasking();
    bool isProcessSwitchingEnabled();
    void killCurrentProcess(isr::registers *regs);
    void interruptTrigger(isr::registers *regs);
    void create_task(void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv, pid_t forced_pid = -1);
    void replace_task(void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv, int replacePid, isr::registers *regs);
    x86_process *getCurrentProcess();
    void waitForProcess(int pid);
    void refresh_current_process_pagerange();
    x86_process *fork_current_process(isr::registers *regs);
    void setProcessSwitching(bool state);
    size_t getProcessCount();
    bool createPageRange(std::vector<process_pagerange> *range, uint32_t max_address = KERNEL_VIRT_ADDRESS);
    void setPageRange(std::vector<process_pagerange> *range);
    void unsetPageRange(std::vector<process_pagerange> *range);
    void freePageRange(std::vector<process_pagerange> *range);
    void printPageRange(std::vector<process_pagerange> *range);
}
