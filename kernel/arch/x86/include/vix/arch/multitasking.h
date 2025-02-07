#pragma once
#include <string>
#include <vector>
#include <vix/arch/common/cpu.h>
#include <vix/arch/isr.h>
#include <vix/arch/paging.h>
#include <vix/config.h>
#include <vix/event.h>
#include <vix/types.h>

namespace multitasking {
    typedef struct {
        uintptr_t phys_base;
        uintptr_t virt_base;
        size_t pages;
        /*
         * static -> code ,bss etc.
         * break -> memory allocated by brk()
         */
        enum class range_type { UNKNOWN, STATIC, BREAK } type;
    } process_pagerange;

    void list_processes();

    void initMultitasking();
    void interruptTrigger();
    void create_task(void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv);
    /*void create_task(void *stackadr,
                     void *codeadr,
                     std::vector<process_pagerange> *pagerange,
                     std::vector<std::string> *argv,
                     struct x86_process::tls_info info,
                     pid_t forced_pid = -1,
                     bool kernel = false);
    void replace_task(void *stackadr,
                      void *codeadr,
                      std::vector<process_pagerange> *pagerange,
                      std::vector<std::string> *argv,
                      struct x86_process::tls_info info,
                      int replacePid,
                      struct arch::full_ctx *regs,
                      bool kernel = false);
    x86_process *getCurrentProcess();
    void waitForProcess(int pid);
    void refresh_current_process_pagerange();
    x86_process *fork_current_process(struct arch::full_ctx *regs);
    void setProcessSwitching(bool state);
    size_t getProcessCount();
    x86_process *get_tid(pid_t tid);*/
    bool createPageRange(std::vector<process_pagerange> *range, uint32_t max_address = KERNEL_VIRT_ADDRESS);
    void setPageRange(std::vector<process_pagerange> *range);
    void unsetPageRange(std::vector<process_pagerange> *range);
    void freePageRange(std::vector<process_pagerange> *range);
    void printPageRange(std::vector<process_pagerange> *range);
}
