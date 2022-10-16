#include <arch/x86/cpubasics.h>
#include <arch/x86/gdt.h>
#include <arch/x86/generic/memory.h>
#include <arch/x86/multitasking.h>
#include <cppstd/vector.h>
#include <debug.h>
#include <log.h>
#include <macros.h>
#include <memory_alloc/memalloc.h>
#include <scheduler.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static std::vector<multitasking::x86_process *> processes;

static schedulers::generic_scheduler_singlethread scheduler;

static bool processSwitchingEnabled = false;

static bool uninitialized = true;

static pid_t pidCounter = 1;

static void cpuidle() {
    while (true) {
        asm volatile("hlt");
    }
}

void multitasking::initMultitasking() {
    scheduler.init((std::vector<schedulers::generic_process *> *)&processes);

    log::log_service("multitasking", "initialized");
    processSwitchingEnabled = true;
    uninitialized = false;
}

bool multitasking::isProcessSwitchingEnabled() {
    return processSwitchingEnabled;
}

/* this function gets passed the top of stack, argv[] must be terminated with a null pointer | returns new stack pointer */
static void *init_empty_stack(void *stackadr, std::vector<std::string> *argv, void *eip = 0, bool kernel = false) { // TODO: make kernel NOT the default
    // get argc
    int argc = argv->size();

    if (argc < 1) {
        LOG_DEBUG("issue: argc too small");
    }

    size_t basesize = 3; // argc, argv null termination and envp null termination
    if (kernel) {
        basesize = 6; // also eip, cs, eflags
    }

    size_t total_size = basesize * 4;

    for (int i = 0; i < argc; i++) {
        total_size += (*argv)[i].size() + 1;
        total_size += 4; // pointer to arg
    }

    stackadr = ((char *)stackadr) - total_size;

    uint32_t *stack = (uint32_t *)stackadr;

    stack[0] = argc; // argc
    stack[0] = 0;

    int nextindex = 1;

    if (kernel) {
        stack[0] = (uint32_t)eip; // EIP
        stack[1] = 8;             // CS?
        stack[2] = 1 << 9;        // EFLAGS, set interrupt bit
        stack[3] = argc;          // argc
        nextindex = 4;
    }

    size_t string_pos = total_size + (argc * 4);

    // build up argv
    for (int i = 0; i < argc; i++) {
        memcpy(((char *)stackadr) + string_pos, (*argv)[i].c_str(), (*argv)[i].size() + 1);
        stack[nextindex++] = (size_t)stackadr + string_pos;
        string_pos += (*argv)[i].size() + 1;
    }
    stack[nextindex++] = 0; // argv null termination
    stack[nextindex++] = 0; // envp null termination

    return stackadr;
}

multitasking::x86_process *multitasking::getCurrentProcess() {
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i]->state == schedulers::generic_process::state::RUNNING) {
            return processes[i];
        }
    }
    return nullptr;
}

static multitasking::x86_process *getProcessByPid(pid_t pid) {
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i]->pid == pid) {
            return processes[i];
        }
    }
    return nullptr;
}

static multitasking::context isr2mt(isr::registers *regs) {
    multitasking::context ret;
    ret.eax = regs->eax;
    ret.ebx = regs->ebx;
    ret.ecx = regs->ecx;
    ret.edx = regs->edx;

    ret.edi = regs->edi;
    ret.esi = regs->esi;

    ret.ebp = regs->ebp;
    ret.esp = regs->esp_kernel;

    ret.eip = regs->eip;

    ret.eflags = regs->eflags;

    return ret;
}

static isr::registers mt2isr(multitasking::context ctx) {
    isr::registers ret;
    ret.eax = ctx.eax;
    ret.ebx = ctx.ebx;
    ret.ecx = ctx.ecx;
    ret.edx = ctx.edx;

    ret.edi = ctx.edi;
    ret.esi = ctx.esi;

    ret.ebp = ctx.ebp;
    ret.esp_kernel = ctx.esp;

    ret.eip = ctx.eip;

    ret.eflags = ctx.eflags;

    return ret;
}

void multitasking::waitForProcess(int pid) {}

void multitasking::refresh_current_process_pagerange() {}

multitasking::x86_process *multitasking::fork_current_process(isr::registers *regs) {
    x86_process *currentProcess = getCurrentProcess();
    currentProcess->registerContext = isr2mt(regs);

    x86_process *new_process = new x86_process;

    new_process->pid = pidCounter++;
    new_process->parent = currentProcess->pid;
    new_process->state = schedulers::generic_process::state::RUNNABLE;
    // new_process->registerContext = currentProcess->registerContext;

    memcpy(&new_process->registerContext, &currentProcess->registerContext, sizeof(context));

    new_process->pages.reserve(currentProcess->pages.size());

    for (size_t i = 0; i < currentProcess->pages.size(); i++) {
        void *newadr = memalloc::page::phys_malloc(currentProcess->pages[i].pages);
        if (!newadr) {
            KERNEL_PANIC("could not allocate memory for fork"); // this shouldn't be a panic later on...
        }
        process_pagerange newPrange;
        newPrange.pages = currentProcess->pages[i].pages;
        newPrange.virt_base = currentProcess->pages[i].virt_base;
        newPrange.phys_base = (size_t)newadr;
        for (size_t j = 0; j < newPrange.pages; j++) {
            paging::copyPhysPage((uint8_t *)newPrange.phys_base + (j * 0x1000), (uint8_t *)currentProcess->pages[i].phys_base + (j * 0x1000));
        }
        new_process->pages.push_back(newPrange);
    }

    processes.push_back(new_process);
    DEBUG_PRINTF("forked %d -> new PID: %d\n", currentProcess->pid, new_process->pid);
    return new_process;
}

void multitasking::killCurrentProcess(isr::registers *regs) {
    x86_process *currentProcess = getCurrentProcess();
    currentProcess->state = schedulers::generic_process::state::KILLED;
    interruptTrigger(regs);
}

void multitasking::create_task(void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv, pid_t forced_pid) {
    setPageRange(pagerange);

    stackadr = init_empty_stack(stackadr, argv, codeadr, true);

    x86_process *kernel_process = new x86_process;
    if (forced_pid != -1) {
        kernel_process->pid = forced_pid;
    } else {
        kernel_process->pid = pidCounter++;
    }
    uint8_t *stack_1 = (uint8_t *)memalloc::single::kmalloc(100);

    kernel_process->registerContext.esp = (uint32_t)stackadr;
    kernel_process->registerContext.eip = (uint32_t)codeadr;
    kernel_process->registerContext.eflags = 1 << 9;

    kernel_process->pages = *pagerange;

    processes.push_back(kernel_process);
    unsetPageRange(pagerange);
}

void multitasking::replace_task(void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv, int replacePid, isr::registers *regs) {
    pid_t PID = -1;
    size_t index = 0;
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i]->pid == replacePid) {
            PID = processes[i]->pid;
            index = i;
        }
    }
    if (PID == -1) {
        return;
    }
    processes[index]->state = schedulers::generic_process::state::REPLACED;
    create_task(stackadr, codeadr, pagerange, argv);
    interruptTrigger(regs);
}

void multitasking::setProcessSwitching(bool state) {
    processSwitchingEnabled = state;
}

size_t multitasking::getProcessCount() {
    size_t count = 0;
    for (size_t i = 0; i < processes.size(); i++) {
        if (!((processes[i]->state == x86_process::state::KILLED) || (processes[i]->state == x86_process::state::REPLACED))) {
            count++;
        }
    }
    return count;
}

uint64_t starttime = 0;

int counter = 0;

// fired every timer interrupt, may be called during an ISR to possibly force a process switch
void multitasking::interruptTrigger(isr::registers *regs) {
    if (unlikely(uninitialized)) {
        starttime = time::getCurrentUnixTime();
        counter = 0;
        return;
    }

    if (counter % 30000 == 0) {
        uint64_t currenttime = time::getCurrentUnixTime();
        uint64_t passedTime = currenttime - starttime;
        DEBUG_PRINTF("timer passed: %u actual passed: %u\n", (uint32_t)(counter / 1000), (uint32_t)passedTime);
    }

    counter++;

    bool switch_;
    size_t switch_index;

    bool old;
    size_t old_index;

    if (!scheduler.tick(&switch_, &switch_index, &old, &old_index)) {
        KERNEL_PANIC("Scheduler error");
    }

    if (switch_) {
        DEBUG_PRINTF("switching process\n");
        if (old) {
            processes[old_index]->registerContext = isr2mt(regs);
            unsetPageRange(&processes[old_index]->pages);
        }

        *regs = mt2isr(processes[switch_index]->registerContext);
        setPageRange(&processes[switch_index]->pages);

        DEBUG_PRINTF("switched process\n");
    }
}

bool multitasking::createPageRange(std::vector<process_pagerange> *range, uint32_t max_address) {
    std::vector<process_pagerange> pages;

    size_t prange_counter = 0;

    uint32_t physAddress = 69420;
    uint32_t lastPhysAddress = 0;
    bool invalidated = true;

    for (uint32_t page = 0; page < (max_address / ARCH_PAGE_SIZE); page++) {
        uint32_t virtadr = page * ARCH_PAGE_SIZE;
        if (paging::is_readable((void *)virtadr)) {
            physAddress = (uint32_t)paging::get_physaddr((void *)virtadr);
            if ((physAddress - ARCH_PAGE_SIZE) != lastPhysAddress || invalidated) {
                pages.push_back({0, 0, 0});
                prange_counter = pages.size() - 1;
                pages[prange_counter].phys_base = physAddress;
                pages[prange_counter].virt_base = virtadr;
                invalidated = false;
            }
            pages[prange_counter].pages++;
            lastPhysAddress = physAddress;
        } else {
            invalidated = true;
        }
    }
    for (size_t i = 0; i < pages.size(); i++) {
        if (pages[i].pages == 0) {
            pages.erase(i);
        }
    }
    *range = pages;
    return true;
}

void multitasking::setPageRange(std::vector<process_pagerange> *range) {
    for (size_t i = 0; i < range->size(); i++) {
        if ((*range)[i].pages > 0) {
            size_t len = (*range)[i].pages;
            void *virt = (void *)((*range)[i].virt_base);
            void *phys = (void *)((*range)[i].phys_base);
            paging::map_page(phys, virt, len, true);
        }
    }
}

void multitasking::unsetPageRange(std::vector<process_pagerange> *range) {
    for (size_t i = 0; i < range->size(); i++) {
        if ((*range)[i].pages > 0) {
            paging::clearPageTables((void *)(*range)[i].virt_base, (*range)[i].pages, true);
        }
    }
}

void multitasking::freePageRange(std::vector<process_pagerange> *range) {
    for (size_t i = 0; i < range->size(); i++) {
        if ((*range)[i].pages > 0) {
            memalloc::page::phys_free((void *)(*range)[i].phys_base);
        }
    }
}

void multitasking::printPageRange(std::vector<process_pagerange> *range) {
    for (size_t i = 0; i < range->size(); i++) {
        if ((*range)[i].pages != 0) {
            printf("pb: 0x%p vb: 0x%p pages: %u\n", (*range)[i].phys_base, (*range)[i].virt_base, (*range)[i].pages);
        }
    }
}
