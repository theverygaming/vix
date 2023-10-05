#pragma once
#include <arch/common/cpu.h>
#include <arch/isr.h>
#include <arch/paging.h>
#include <config.h>
#include <event.h>
#include <scheduler.h>
#include <string>
#include <types.h>
#include <vector>

namespace multitasking {
    extern event_dispatcher<pid_t> process_deth_events;

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

    class x86_process : public schedulers::generic_process {
    public:
        struct arch::full_ctx reg_ctx;
        std::vector<process_pagerange> pages;
        uintptr_t brk_start = 0;
        struct tls_info {
            void *tlsdata = nullptr;
            size_t tlsdata_size;
            size_t tls_size; // entire TLS, including null bytes
        } tlsinfo;
    };

    void list_processes();

    void initMultitasking();
    bool isProcessSwitchingEnabled();
    void killCurrentProcess(struct arch::full_ctx *regs);
    void interruptTrigger(struct arch::full_ctx *regs);
    void create_task(void *stackadr,
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
    x86_process *get_tid(pid_t tid);
    void reschedule(struct arch::full_ctx *regs);
    bool createPageRange(std::vector<process_pagerange> *range, uint32_t max_address = KERNEL_VIRT_ADDRESS);
    void setPageRange(std::vector<process_pagerange> *range);
    void unsetPageRange(std::vector<process_pagerange> *range);
    void freePageRange(std::vector<process_pagerange> *range);
    void printPageRange(std::vector<process_pagerange> *range);
}
