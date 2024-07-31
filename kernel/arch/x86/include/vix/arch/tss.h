#pragma once
#include <vix/types.h>

namespace tss {
    struct __attribute__((packed)) tss_protectedmode {
        uint16_t link;
        uint16_t _reserved1;

        uint32_t esp0; // ring 0

        uint16_t ss0;
        uint16_t _reserved2;

        uint32_t esp1; // ring 1

        uint16_t ss1;
        uint16_t _reserved3;

        uint32_t esp2; // ring 2

        uint16_t ss2;
        uint16_t _reserved4;

        uint32_t cr3;

        uint32_t eip;

        uint32_t eflags;

        uint32_t eax;

        uint32_t ecx;

        uint32_t edx;

        uint32_t ebx;

        uint32_t esp; // ring 3

        uint32_t ebp;

        uint32_t esi;

        uint32_t edi;

        uint16_t es;
        uint16_t _reserved5;

        uint16_t cs;
        uint16_t _reserved6;

        uint16_t ss; // ring 3
        uint16_t _reserved7;

        uint16_t ds;
        uint16_t _reserved8;

        uint16_t fs;
        uint16_t _reserved9;

        uint16_t gs;
        uint16_t _reserved10;

        uint16_t ldtr;
        uint16_t _reserved11;

        uint16_t _reserved12;
        uint16_t iopb;

        uint32_t ssp;
    };

    extern struct tss_protectedmode tss_entry;
}
