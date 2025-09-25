#include <vix/panic.h>
#include <string.h>
#include <vector>
#include <vix/abi/linux/errno.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/common/paging.h>
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
#include <vix/mm/mm.h>
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

void multitasking::initMultitasking() {
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
    arch::vmm::load_pt(sched::mytask()->task_arch.pt);
    x86_load_cpu_full_ctx((struct arch::full_ctx *)sched::mytask()->data);
}

#ifdef CONFIG_ARCH_HAS_PAGING
void multitasking::create_task(void *stackadr, void *codeadr, arch::vmm::pt_t pt, std::vector<std::string> *argv) {
    arch::vmm::pt_t prev_pt = arch::vmm::get_active_pt();
    arch::vmm::load_pt(pt);

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

    arch::vmm::load_pt(prev_pt);
    struct sched::task t = sched::init_thread(user_thread_launch, ctx);
    t.task_arch.pt = pt;
    t.task_arch.is_ring_3 = true;
    sched::start_thread(t);
}
#endif

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
