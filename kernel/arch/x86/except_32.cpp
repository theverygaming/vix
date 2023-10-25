#include <arch/common/cpu.h>
#include <arch/multitasking.h>
#include <panic.h>
#include <symbols.h>

struct stackframe {
    struct stackframe *ebp;
    uint32_t eip;
};

static void st_print_ip(uint32_t ip) {
    std::pair<const char *, uintptr_t> sr = syms::find_func_sym(ip);
    if (sr.first == nullptr) {
        return;
    }
    kprintf(KP_ALERT, "TRACE: [0x%p] %s+0x%p\n", ip, sr.first, ip - sr.second);
}

static void do_stack_trace(uint32_t ebp) {
    struct stackframe *p = (struct stackframe *)ebp;
    while (p != nullptr) {
        st_print_ip(p->eip);
        p = p->ebp;
        if ((uint32_t)p->ebp < 0xC0000000) {
            break;
        }
    }
}

extern "C" void handle_x86_except(struct arch::full_ctx *regs) {
    switch (regs->interrupt) {
    case 14: {
        uint32_t fault_address;
        asm volatile("mov %%cr2, %0" : "=r"(fault_address)); // get address page fault occurred at
        int present = !(regs->error_code & 0x1);             // page not present
        int rw = regs->error_code & 0x2;                     // is caused by write
        int us = regs->error_code & 0x4;                     // user or kernel fault?
        int reserved = regs->error_code & 0x8;               // reserved bt fuckup?
        int id = regs->error_code & 0x10;                    // instruction access or data?

        // Output an error message.
        kprintf(KP_ALERT, "isr: Page fault! (\n");
        if (present) {
            kprintf(KP_ALERT, "isr: non-present\n");
        }
        if (rw) {
            kprintf(KP_ALERT, "isr: read-only\n");
        }
        if (us) {
            kprintf(KP_ALERT, "isr: user-mode\n");
        }
        if (reserved) {
            kprintf(KP_ALERT, "isr: reserved\n");
        }
        if (id) {
            kprintf(KP_ALERT, "isr: instruction-fetch\n");
        }
        kprintf(KP_ALERT, "isr: ) at 0x%p\n", fault_address);
        kprintf(KP_ALERT, "isr: Error code: 0x%p\n", regs->error_code);
        kprintf(KP_ALERT,
                "isr: eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n",
                regs->eax,
                regs->ebx,
                regs->ecx,
                regs->edx,
                regs->esi,
                regs->edi,
                regs->esp,
                regs->ebp,
                regs->eip);
        if ((regs->eip >= KERNEL_VIRT_ADDRESS) && (regs->eip < KERNEL_VIRT_ADDRESS + KERNEL_MEMORY_END_OFFSET)) {
            st_print_ip(regs->eip);
            do_stack_trace(regs->ebp);
            KERNEL_PANIC("kernel page fault");
        }
        kprintf(KP_ALERT, "isr: Killing current process\n");
        st_print_ip(regs->eip);
        do_stack_trace(regs->ebp);
        KERNEL_PANIC("PANIC_ON_PROGRAM_FAULT");
    }
    default: {
        kprintf(KP_EMERG,
                "isr: eax: 0x%p ebx: 0x%p ecx: 0x%p edx: 0x%p\nesi: 0x%p edi: 0x%p esp: 0x%p ebp: 0x%p eip: 0x%p\n",
                regs->eax,
                regs->ebx,
                regs->ecx,
                regs->edx,
                regs->esi,
                regs->edi,
                regs->esp,
                regs->ebp,
                regs->eip);
        kprintf(KP_EMERG, "unhandled exception %u\n", (unsigned int)regs->interrupt);
        st_print_ip(regs->eip);
        do_stack_trace(regs->ebp);
        KERNEL_PANIC("unhandled exception");
    }
    }
}
