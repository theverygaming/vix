#include <arch/cpubasics.h>
#include <arch/gdt.h>
#include <arch/generic/memory.h>
#include <arch/multitasking.h>
#include <arch/syscalls.h>
#include <cppstd/vector.h>
#include <debug.h>
#include <errno.h>
#include <log.h>
#include <macros.h>
#include <mm/memalloc.h>
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
static void *init_empty_stack(void *stackadr, std::vector<std::string> *argv, void *eip = 0, bool kernel = false) {
    stackadr = ((uint8_t *)stackadr) - (10 * 4);
    memset(stackadr, 0, 10 * 4);
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

    stackadr = ((uint8_t *)stackadr) - total_size;

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
    } else {
        stack[0] = argc; // argc
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

    ret.ds = regs->ds;
    ret.es = regs->es;
    ret.fs = regs->fs;
    ret.gs = regs->gs;
    ret.ss = regs->ss_user;

    ret.cs = regs->cs;

    ret.edi = regs->edi;
    ret.esi = regs->esi;

    ret.ebp = regs->ebp;
    // ret.esp = regs->esp_kernel;
    ret.esp = regs->esp_user;

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

    ret.ds = ctx.ds;
    ret.es = ctx.es;
    ret.fs = ctx.fs;
    ret.gs = ctx.gs;
    ret.ss_user = ctx.ss;

    ret.cs = ctx.cs;

    ret.edi = ctx.edi;
    ret.esi = ctx.esi;

    ret.ebp = ctx.ebp;
    // ret.esp_kernel = ctx.esp;
    ret.esp_user = ctx.esp;

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

void multitasking::create_task(void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv, struct x86_process::tls_info info, pid_t forced_pid) {
    setPageRange(pagerange);

    stackadr = init_empty_stack(stackadr, argv, codeadr, false);

    x86_process *kernel_process = new x86_process;
    if (forced_pid != -1) {
        kernel_process->pid = forced_pid;
    } else {
        kernel_process->pid = pidCounter++;
    }
    uint8_t *stack_1 = (uint8_t *)mm::kmalloc(100);

    kernel_process->registerContext.cs = (3 * 8) | 3;
    kernel_process->registerContext.ds = (4 * 8) | 3;
    kernel_process->registerContext.es = (4 * 8) | 3;
    kernel_process->registerContext.fs = (4 * 8) | 3;
    kernel_process->registerContext.gs = (4 * 8) | 3;
    kernel_process->registerContext.ss = (4 * 8) | 3;

    kernel_process->registerContext.esp = (uint32_t)stackadr;
    kernel_process->registerContext.eip = (uint32_t)codeadr;
    kernel_process->registerContext.eflags = 1 << 9;

    kernel_process->pages = *pagerange;
    kernel_process->tlsinfo = info;

    // quick hack: find out where program break is
    for (size_t i = 0; i < pagerange->size(); i++) {
        if ((*pagerange)[i].type == process_pagerange::range_type::BREAK) {
            kernel_process->brk_start = (*pagerange)[i].virt_base;
        }
    }
    if (kernel_process->brk_start == 0) {
        KERNEL_PANIC("unable to find break");
    }

    processes.push_back(kernel_process);
    unsetPageRange(pagerange);
}

void multitasking::replace_task(
    void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv, struct x86_process::tls_info info, int replacePid, isr::registers *regs) {
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
    create_task(stackadr, codeadr, pagerange, argv, info);
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

// uint64_t starttime = 0;

// int counter = 0;

// fired every timer interrupt, may be called during an ISR to possibly force a process switch
void multitasking::interruptTrigger(isr::registers *regs) {
    if (unlikely(uninitialized)) {
        // starttime = time::getCurrentUnixTime();
        // counter = 0;
        return;
    }

    /*if (counter % 30000 == 0) {
        uint64_t currenttime = time::getCurrentUnixTime();
        uint64_t passedTime = currenttime - starttime;
        DEBUG_PRINTF("timer passed: %u actual passed: %u\n", (uint32_t)(counter / 1000), (uint32_t)passedTime);
    }

    counter++;*/

    bool switch_;
    size_t switch_index;

    bool old;
    size_t old_index;

    if (!scheduler.tick(&switch_, &switch_index, &old, &old_index)) {
        KERNEL_PANIC("Scheduler error");
    }

    if (switch_) {
        if (old) {
            processes[old_index]->registerContext = isr2mt(regs);
            unsetPageRange(&processes[old_index]->pages);
        }

        *regs = mt2isr(processes[switch_index]->registerContext);
        setPageRange(&processes[switch_index]->pages);

        DEBUG_PRINTF("switched process to %d\n", processes[switch_index]->pid);
    }
}

// from linux: /include/uapi/linux/sched.h
#define CLONE_VM             0x00000100 /* set if VM shared between processes */
#define CLONE_FS             0x00000200 /* set if fs info shared between processes */
#define CLONE_FILES          0x00000400 /* set if open files shared between processes */
#define CLONE_SIGHAND        0x00000800 /* set if signal handlers and blocked signals shared */
#define CLONE_PTRACE         0x00002000 /* set if we want to let tracing continue on the child too */
#define CLONE_VFORK          0x00004000 /* set if the parent wants the child to wake it up on mm_release */
#define CLONE_PARENT         0x00008000 /* set if we want to have the same parent as the cloner */
#define CLONE_THREAD         0x00010000 /* Same thread group? */
#define CLONE_NEWNS          0x00020000 /* New mount namespace group */
#define CLONE_SYSVSEM        0x00040000 /* share system V SEM_UNDO semantics */
#define CLONE_SETTLS         0x00080000 /* create a new TLS for the child */
#define CLONE_PARENT_SETTID  0x00100000 /* set the TID in the parent */
#define CLONE_CHILD_CLEARTID 0x00200000 /* clear the TID in the child */
#define CLONE_DETACHED       0x00400000 /* Unused, ignored */
#define CLONE_UNTRACED       0x00800000 /* set if the tracing process can't force CLONE_PTRACE on this clone */
#define CLONE_CHILD_SETTID   0x01000000 /* set the TID in the child */
#define CLONE_NEWUTS         0x04000000 /* New utsname namespace */
#define CLONE_NEWIPC         0x08000000 /* New ipc namespace */
#define CLONE_NEWUSER        0x10000000 /* New user namespace */
#define CLONE_NEWPID         0x20000000 /* New pid namespace */
#define CLONE_NEWNET         0x40000000 /* New network namespace */
#define CLONE_IO             0x80000000 /* Clone io context */

uint32_t sys_clone(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t _flags, uint32_t _stack, uint32_t _parent_tid, uint32_t _tls, uint32_t _child_tid, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_clone");
    unsigned long flags = _flags;
    void *stack = (void *)_stack;
    int *parent_tid = (int *)_parent_tid;
    unsigned long tls = (unsigned long)_tls;
    int *child_tid = (int *)_child_tid;
    DEBUG_PRINTF("clone: flags -> %u stack -> 0x%p tls -> 0x%p\n", flags, stack, tls);
    if (stack == nullptr) {
        return -EINVAL;
    }

#define CHECKFLAG(flag)              \
    do {                             \
        if (flags & flag) {          \
            DEBUG_PRINTF(#flag " "); \
        }                            \
    } while (0)
    DEBUG_PRINTF("clone flags: ");
    CHECKFLAG(CLONE_VM);
    CHECKFLAG(CLONE_FS);
    CHECKFLAG(CLONE_FILES);
    CHECKFLAG(CLONE_SIGHAND);
    CHECKFLAG(CLONE_PTRACE);
    CHECKFLAG(CLONE_VFORK);
    CHECKFLAG(CLONE_PARENT);
    CHECKFLAG(CLONE_THREAD);
    CHECKFLAG(CLONE_NEWNS);
    CHECKFLAG(CLONE_SYSVSEM);
    CHECKFLAG(CLONE_SETTLS);
    CHECKFLAG(CLONE_PARENT_SETTID);
    CHECKFLAG(CLONE_CHILD_CLEARTID);
    CHECKFLAG(CLONE_DETACHED);
    CHECKFLAG(CLONE_UNTRACED);
    CHECKFLAG(CLONE_CHILD_SETTID);
    CHECKFLAG(CLONE_NEWUTS);
    CHECKFLAG(CLONE_NEWIPC);
    CHECKFLAG(CLONE_NEWUSER);
    CHECKFLAG(CLONE_NEWPID);
    CHECKFLAG(CLONE_NEWNET);
    CHECKFLAG(CLONE_IO);
    DEBUG_PRINTF("\n");

    /*
     * musl uses
     * CLONE_VM -- same physical memory space // TODO: "any memory mapping or unmapping performed with mmap(2) or munmap(2) by the child or calling process also affects the other process."
     * CLONE_FS -- synced chroot, chdir etc. // TODO
     * CLONE_FILES -- synced fd table // TODO
     * CLONE_SIGHAND -- synced signal handlers // TODO
     * CLONE_THREAD -- same thread group as parent
     * CLONE_SYSVSEM -- idk
     * CLONE_SETTLS -- // TODO !!!!
     * CLONE_PARENT_SETTID
     * CLONE_CHILD_CLEARTID
     * CLONE_DETACHED // can be _mostly_ ignored since linux 2.6.2 // TODO: look into this
     */

    if (!(flags & CLONE_VM)) {
        return -EINVAL;
    }

    if (!(flags & CLONE_THREAD)) {
        return -EINVAL;
    }

    if (flags & CLONE_CHILD_CLEARTID) {
        // TODO: something something futex wakeup
        // will wake up parent futex and set child_tid to zero when child exits
    }

    multitasking::x86_process *currentProcess = multitasking::getCurrentProcess();
    currentProcess->registerContext = isr2mt(regs);

    multitasking::x86_process *new_process = new multitasking::x86_process;

    new_process->pid = pidCounter++;

    if (flags & CLONE_PARENT_SETTID) {
        *parent_tid = new_process->pid;
    }

    new_process->parent = currentProcess->pid;
    new_process->state = schedulers::generic_process::state::RUNNABLE;
    // new_process->registerContext = currentProcess->registerContext;

    memcpy(&new_process->registerContext, &currentProcess->registerContext, sizeof(multitasking::context));
    new_process->registerContext.esp = (uintptr_t)stack;

    new_process->registerContext.eax = 0;

    new_process->pages = currentProcess->pages;

    processes.push_back(new_process);
    DEBUG_PRINTF("cloned %d -> new TID: %d\n", currentProcess->pid, new_process->pid);

    return new_process->pid;
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
