#include <vix/types.h>
#include <vix/symbols.h>
#include <vix/kprintf.h>
#include <vix/debug.h>

struct stackframe {
    struct stackframe *rfp;
    uint32_t lr;
};

static void st_print_ip(uintptr_t ip) {
    std::pair<const char *, uintptr_t> sr = syms::find_func_sym(ip);
    if (sr.first == nullptr) {
        kprintf(KP_ALERT, "trace: [0x%p] Failed to resolve symbol\n", ip);
        return;
    }
    kprintf(KP_ALERT, "trace: [0x%p] %s+0x%p\n", ip, sr.first, ip - sr.second);
}

static void do_stack_trace(uintptr_t rfp) {
    struct stackframe *p = (struct stackframe *)rfp;
    while (p != nullptr) {
        DEBUG_PRINTF("trace: p: 0x%p rfp: 0x%p lr: 0x%p\n", p, p->lr, p->rfp);
        st_print_ip(p->lr);
        p = p->rfp;
    }
}

void funnyarch_stack_trace() {
    uintptr_t rfp;
    asm volatile("mov %0, rfp" : "=r"(rfp)::);
    do_stack_trace(rfp);
}
