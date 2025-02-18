#include <string.h>
#include <vector>
#include <vix/abi/linux/errno.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/gdt.h>
#include <vix/arch/multitasking.h>
#include <vix/arch/tss.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/interrupts.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/mm/pmm.h>
#include <vix/sched.h>
#include <vix/stdio.h>
#include <vix/time.h>

static volatile bool uninitialized = true;

void multitasking::list_processes() {
    for (auto it = sched::sched_readyqueue.begin(); it != sched::sched_readyqueue.end(); it++) {
        printf("PID: %d state: %c\n", it->pid, (it->state == sched::task::state::RUNNING) ? 'R' : 'S');
    }
}

static void cpuidle() {
    while (true) {
        asm volatile("hlt");
    }
}

void multitasking::initMultitasking() {
    /*scheduler.init((std::vector<schedulers::generic_process *> *)&processes,
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
    */
    kprintf(KP_INFO, "multitasking: initialized\n");
    uninitialized = false;
}

/* this function gets passed the top of stack, argv[] must be terminated with a null pointer | returns new stack pointer */
static void *init_user_stack(void *stackadr, std::vector<std::string> *argv, void *eip = 0, bool kernel = false) {
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

extern "C" void x86_load_cpu_full_ctx(struct arch::full_ctx *ctx);
static void user_thread_launch() {
    volatile int test = 5;
    kprintf(KP_INFO,
            "hi from user thread(PID %d) stack: 0x%p stack top: 0x%p\n",
            sched::mytask()->pid,
            &test,
            sched::mytask()->task_arch.kernel_stack_top);
    tss::tss_entry.ss0 = GDT_KERNEL_DATA;
    tss::tss_entry.esp0 = (uintptr_t)sched::mytask()->task_arch.kernel_stack_top;
    multitasking::setPageRange(&sched::mytask()->task_arch.pages);
    x86_load_cpu_full_ctx((struct arch::full_ctx *)sched::mytask()->data);
}

void multitasking::create_task(void *stackadr, void *codeadr, std::vector<process_pagerange> *pagerange, std::vector<std::string> *argv) {
    setPageRange(pagerange);

    stackadr = init_user_stack(stackadr, argv, codeadr, false);

    struct arch::full_ctx *ctx = (struct arch::full_ctx *)mm::kmalloc(sizeof(struct arch::full_ctx));

    uint16_t cs = GDT_USER_CODE | 3;
    uint16_t ds = GDT_USER_DATA | 3;

    ctx->cs = cs;
    ctx->ds = ds;
    ctx->es = ds;
    ctx->fs = ds;
    ctx->gs = ds;
    ctx->ss = ds;

    ctx->esp = (uintptr_t)stackadr;
    ctx->eip = (uintptr_t)codeadr;
    ctx->eflags = 1 << 9;

    unsetPageRange(pagerange);
    struct sched::task t = sched::init_thread(user_thread_launch, ctx);
    t.task_arch.pages = *pagerange;
    t.task_arch.is_ring_3 = true;
    sched::start_thread(t);
}

/*
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
        void *newadr = mm::pmm::alloc_contiguous(currentProcess->pages[i].pages);
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
        stackadr = init_user_stack(stackadr, argv, codeadr, false);
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
*/

// called on every timer interrupt
void multitasking::interruptTrigger() {
    if (unlikely(uninitialized)) {
        return;
    }
    // maybe these could be assertions
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED);
        KERNEL_PANIC("interrupts on in isr");
    }
    if (sched::mytask()->task_arch.is_ring_3) {
        // check if the TSS ESP0 is correct (this check can be removed later, just for debugging purposes -- maybe make it an assertion)
        if (tss::tss_entry.esp0 != 0 && (tss::tss_entry.esp0 != (uintptr_t)sched::mytask()->task_arch.kernel_stack_top)) {
            KERNEL_PANIC("invalid TSS ESP0  - got 0x%p expected 0x%p", tss::tss_entry.esp0, sched::mytask()->task_arch.kernel_stack_top);
        }
    }
    sched::yield();
    if (sched::mytask()->task_arch.is_ring_3) {
        // check if the TSS ESP0 is correct (this check can be removed later, just for debugging purposes -- maybe make it an assertion?)
        if (tss::tss_entry.esp0 != 0 && (tss::tss_entry.esp0 != (uintptr_t)sched::mytask()->task_arch.kernel_stack_top)) {
            KERNEL_PANIC("invalid TSS ESP0  - got 0x%p expected 0x%p", tss::tss_entry.esp0, sched::mytask()->task_arch.kernel_stack_top);
        }
    }
    if (arch::get_interrupt_state() != arch::INTERRUPT_STATE_DISABLED) {
        arch::set_interrupt_state(arch::INTERRUPT_STATE_DISABLED);
        KERNEL_PANIC("interrupts on in isr");
    }
}

bool multitasking::createPageRange(std::vector<process_pagerange> *range, uint32_t max_address) {
    std::vector<process_pagerange> pages;

    size_t prange_counter = 0;

    uintptr_t physAddress = 69420;
    uintptr_t lastPhysAddress = 0;
    bool invalidated = true;

    for (uint32_t page = 0; page < (max_address / CONFIG_ARCH_PAGE_SIZE); page++) {
        uint32_t virtadr = page * CONFIG_ARCH_PAGE_SIZE;
        if (paging::is_readable((void *)virtadr)) {
            physAddress = (uintptr_t)paging::get_physaddr((void *)virtadr);
            if ((physAddress - CONFIG_ARCH_PAGE_SIZE) != lastPhysAddress || invalidated) {
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
            mm::pmm::free_contiguous((void *)(*range)[i].phys_base, (*range)[i].pages);
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
