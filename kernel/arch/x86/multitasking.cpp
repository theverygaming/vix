#include <abi/linux/errno.h>
#include <arch/common/cpu.h>
#include <arch/cpubasics.h>
#include <arch/gdt.h>
#include <arch/generic/memory.h>
#include <arch/multitasking.h>
#include <arch/syscalls_32.h>
#include <debug.h>
#include <interrupts.h>
#include <kprintf.h>
#include <macros.h>
#include <mm/kmalloc.h>
#include <mm/phys.h>
#include <sched.h>
#include <scheduler.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

event_dispatcher<pid_t> multitasking::process_deth_events;

static std::vector<multitasking::x86_process *> processes;

static schedulers::generic_scheduler_singlethread scheduler;

static bool processSwitchingEnabled = false;

static volatile bool uninitialized = true;

static pid_t pidCounter = 1;

void multitasking::list_processes() {
    for (size_t i = 0; i < processes.size(); i++) {
        printf("[%u] tgid: %d tid: %d state: %d\n", i, processes[i]->tgid, processes[i]->tid, processes[i]->state);
    }
    for (auto it = sched::sched_readyqueue.begin(); it != sched::sched_readyqueue.end(); it++) {
        printf("PID: %d state: %c\n", it->pid, (it->state == sched::task::state::RUNNING) ? 'R' : 'S');
    }
}

static void cpuidle() {
    while (true) {
        asm volatile("hlt");
    }
}

static void(load_process)(multitasking::x86_process *proc, void *ctx);
static void(unload_process)(multitasking::x86_process *proc, void *ctx);

void multitasking::initMultitasking() {
    scheduler.init((std::vector<schedulers::generic_process *> *)&processes,
                   (void (*)(schedulers::generic_process *, void *))load_process,
                   (void (*)(schedulers::generic_process *, void *))unload_process);

    // TODO: run idle process only when required
    x86_process *idle = new x86_process;
    idle->tgid = 0;
    idle->reg_ctx.cs = GDT_KERNEL_CODE;
    idle->reg_ctx.ds = GDT_KERNEL_DATA;
    idle->reg_ctx.es = GDT_KERNEL_DATA;
    idle->reg_ctx.fs = GDT_KERNEL_DATA;
    idle->reg_ctx.gs = GDT_KERNEL_DATA;
    idle->reg_ctx.ss = GDT_KERNEL_DATA;
    idle->reg_ctx.esp = 0; // TODO: investigate kernel task stack pointer bug
    idle->reg_ctx.eip = (uintptr_t)cpuidle;
    idle->reg_ctx.eflags = 1 << 9;
    processes.push_back(idle);

    kprintf(KP_INFO, "multitasking: initialized\n");
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
        DEBUG_PRINTF("issue: argc too small\n");
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
        stack[0] = (uintptr_t)eip; // EIP
        stack[1] = 8;              // CS?
        stack[2] = 1 << 9;         // EFLAGS, set interrupt bit
        stack[3] = argc;           // argc
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
        if (processes[i]->tgid == pid) {
            return processes[i];
        }
    }
    return nullptr;
}

void multitasking::waitForProcess(int pid) {}

void multitasking::refresh_current_process_pagerange() {}

multitasking::x86_process *multitasking::fork_current_process(struct arch::full_ctx *regs) {
    x86_process *currentProcess = getCurrentProcess();
    currentProcess->reg_ctx = *regs;

    x86_process *new_process = new x86_process;

    new_process->tgid = pidCounter++;
    new_process->parent = currentProcess->tgid;
    new_process->state = schedulers::generic_process::state::RUNNABLE;
    // new_process->reg_ctx = currentProcess->reg_ctx;

    memcpy(&new_process->reg_ctx, &currentProcess->reg_ctx, sizeof(new_process->reg_ctx));

    new_process->pages.reserve(currentProcess->pages.size());

    for (size_t i = 0; i < currentProcess->pages.size(); i++) {
        void *newadr = mm::phys::phys_malloc(currentProcess->pages[i].pages);
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
    DEBUG_PRINTF("forked %d -> new PID: %d\n", currentProcess->tgid, new_process->tgid);
    return new_process;
}

void multitasking::killCurrentProcess(struct arch::full_ctx *regs) {
    x86_process *currentProcess = getCurrentProcess();
    process_deth_events.dispatch(currentProcess->tgid);
    freePageRange(&currentProcess->pages);
    currentProcess->state = schedulers::generic_process::state::KILLED;
    interruptTrigger(regs);
}

multitasking::x86_process *created_x86_proc = nullptr;
void multitasking::create_task(void *stackadr,
                               void *codeadr,
                               std::vector<process_pagerange> *pagerange,
                               std::vector<std::string> *argv,
                               struct x86_process::tls_info info,
                               pid_t forced_pid,
                               bool kernel) {
    setPageRange(pagerange);

    if (!kernel) {
        stackadr = init_empty_stack(stackadr, argv, codeadr, false);
    }

    x86_process *new_process = new x86_process;
    created_x86_proc = new_process;
    if (forced_pid != -1) {
        new_process->tgid = forced_pid;
    } else {
        new_process->tgid = pidCounter++;
    }

    uint16_t cs = GDT_USER_CODE | 3;
    uint16_t ds = GDT_USER_DATA | 3;
    if (kernel) {
        cs = GDT_KERNEL_CODE;
        ds = GDT_KERNEL_DATA;
    }

    new_process->reg_ctx.cs = cs;
    new_process->reg_ctx.ds = ds;
    new_process->reg_ctx.es = ds;
    new_process->reg_ctx.fs = ds;
    new_process->reg_ctx.gs = ds;
    new_process->reg_ctx.ss = ds;

    new_process->reg_ctx.esp = (uintptr_t)stackadr;
    new_process->reg_ctx.eip = (uintptr_t)codeadr;
    new_process->reg_ctx.eflags = 1 << 9;

    new_process->pages = *pagerange;

    if (!kernel) {
        new_process->tlsinfo = info;
    }

    if (!kernel) {
        // quick hack: find out where program break is
        for (size_t i = 0; i < pagerange->size(); i++) {
            if ((*pagerange)[i].type == process_pagerange::range_type::BREAK) {
                new_process->brk_start = (*pagerange)[i].virt_base;
            }
        }
        if (new_process->brk_start == 0) {
            KERNEL_PANIC("unable to find break");
        }
    }

    processes.push_back(new_process);
    unsetPageRange(pagerange);
}

void multitasking::replace_task(void *stackadr,
                                void *codeadr,
                                std::vector<process_pagerange> *pagerange,
                                std::vector<std::string> *argv,
                                struct x86_process::tls_info info,
                                int replacePid,
                                struct arch::full_ctx *regs,
                                bool kernel) {
    pid_t pid = -1;
    size_t index = 0;
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i]->tgid == replacePid) {
            pid = processes[i]->tgid;
            index = i;
        }
    }
    if (pid == -1) {
        return;
    }
    processes[index]->state = schedulers::generic_process::state::REPLACED;
    create_task(stackadr, codeadr, pagerange, argv, info, replacePid, kernel);
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

multitasking::x86_process *multitasking::get_tid(pid_t tid) {
    for (size_t i = 0; i < processes.size(); i++) {
        if (!((processes[i]->state == x86_process::state::KILLED) || (processes[i]->state == x86_process::state::REPLACED)) &&
            processes[i]->tgid == tid) {
            return processes[i];
        }
    }
    return nullptr;
}

void multitasking::reschedule(struct arch::full_ctx *regs) {
    interruptTrigger(regs);
}

static void(load_process)(multitasking::x86_process *proc, void *ctx) {
    struct arch::full_ctx *regs = (struct arch::full_ctx *)ctx;
    *regs = proc->reg_ctx;
    setPageRange(&proc->pages);
    if (proc->tgid != 0) {
        // DEBUG_PRINTF_INSANE("loaded %d\n", proc->tgid);
    }
}

static void(unload_process)(multitasking::x86_process *proc, void *ctx) {
    struct arch::full_ctx *regs = (struct arch::full_ctx *)ctx;
    proc->reg_ctx = *regs;
    unsetPageRange(&proc->pages);
    if (proc->tgid != 0) {
        // DEBUG_PRINTF_INSANE("unloaded %d\n", proc->tgid);
    }
}

// fired every timer interrupt, may be called during an ISR to possibly force a process switch
void multitasking::interruptTrigger(struct arch::full_ctx *regs) {
    if (unlikely(uninitialized)) {
        return;
    }
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED);
        KERNEL_PANIC("interrupts on in isr");
    }
    sched::yield();
    return;

    if (!unlikely(scheduler.tick(regs))) {
        list_processes();
        KERNEL_PANIC("Scheduler error");
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

uint32_t sys_clone(struct arch::full_ctx *regs,
                   int *syscall_ret,
                   uint32_t,
                   uint32_t _flags,
                   uint32_t _stack,
                   uint32_t _parent_tid,
                   uint32_t _tls,
                   uint32_t _child_tid,
                   uint32_t) {
    *syscall_ret = 1;
    DEBUG_PRINTF("syscall: sys_clone\n");
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
    currentProcess->reg_ctx = *regs;

    multitasking::x86_process *new_process = new multitasking::x86_process;

    new_process->tgid = pidCounter++;

    if (flags & CLONE_PARENT_SETTID) {
        *parent_tid = new_process->tgid;
    }

    new_process->parent = currentProcess->tgid;
    new_process->state = schedulers::generic_process::state::RUNNABLE;
    // new_process->reg_ctx = currentProcess->reg_ctx;

    memcpy(&new_process->reg_ctx, &currentProcess->reg_ctx, sizeof(currentProcess->reg_ctx));
    new_process->reg_ctx.esp = (uintptr_t)stack;

    new_process->reg_ctx.eax = 0;

    new_process->pages = currentProcess->pages;

    processes.push_back(new_process);
    DEBUG_PRINTF("cloned %d -> new TID: %d\n", currentProcess->tgid, new_process->tgid);

    return new_process->tgid;
}

bool multitasking::createPageRange(std::vector<process_pagerange> *range, uint32_t max_address) {
    std::vector<process_pagerange> pages;

    size_t prange_counter = 0;

    uintptr_t physAddress = 69420;
    uintptr_t lastPhysAddress = 0;
    bool invalidated = true;

    for (uint32_t page = 0; page < (max_address / ARCH_PAGE_SIZE); page++) {
        uint32_t virtadr = page * ARCH_PAGE_SIZE;
        if (paging::is_readable((void *)virtadr)) {
            physAddress = (uintptr_t)paging::get_physaddr((void *)virtadr);
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
            mm::phys::phys_free((void *)(*range)[i].phys_base, (*range)[i].pages);
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
