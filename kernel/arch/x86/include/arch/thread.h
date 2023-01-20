#pragma once
#include <arch/multitasking.h>
#include <config.h>
#include <mm/kmalloc.h>
#include <panic.h>
#include <types.h>

namespace std {
    class thread {
    public:
        thread(void (*func)(void *arg), void *arg) {
#ifdef CONFIG_ENABLE_KERNEL_32
            int stacklen = 10000; // passed from user later
            uint32_t *stack = (uint32_t *)((uint8_t *)mm::kmalloc_aligned(stacklen * sizeof(uintptr_t), sizeof(uintptr_t)) + (stacklen * sizeof(uintptr_t)));
            stack--;
            *stack = (uint32_t)arg;
            struct multitasking::x86_process::tls_info tlsinfo = {.tlsdata = nullptr};
            std::vector<multitasking::process_pagerange> pageranges;
            multitasking::create_task(stack, (void *)func, &pageranges, nullptr, tlsinfo, -1, true);
#endif
        }

    private:
    };
}
