#pragma once
#include <vix/types.h>

namespace arch {
    struct __attribute__((packed)) full_ctx {
        // pushed & popped by us
        uint64_t rbx;
        uint64_t rdx;
        uint64_t rsi;
        uint64_t rbp;
        uint64_t r8;
        uint64_t r9;
        uint64_t r10;
        uint64_t r12;
        uint64_t r13;
        uint64_t r14;
        uint64_t r15;
        uint64_t fs_base;
        uint64_t gs_base;

        // only pushed by monitor, not popped
        uint64_t interrupt_meta4;
        uint64_t interrupt_meta3;
        uint64_t interrupt_meta2;
        uint64_t interrupt_meta1;
        uint64_t interrupt_code;

        // pushed & popped by monitor
        uint64_t monitor_flags;
        uint64_t rdi;
        uint64_t rax;
        uint64_t r11;
        uint64_t rcx;
        uint64_t rflags;
        uint64_t cs;
        uint64_t rip;
        uint64_t rsp;
    };

    struct __attribute__((packed)) ctx {
        uint64_t r12;
        uint64_t r13;
        uint64_t r14;
        uint64_t r15;
        uint64_t rbx;
        uint64_t rbp;
        uint64_t rip;
    };
}
