#include "elf.h"
#include "memalloc.h"
#include "multitasking.h"
#include <arch/x86/paging.h>
#include "stdio.h"
#include "stdlib.h"

void elf::load_program(void *ELF_baseadr) {
    ElfHeader header;
    ElfProgramHeader pHeader;

    memcpy((char *)&header, (char *)ELF_baseadr, sizeof(ElfHeader));

    uint32_t max = 0;
    uint32_t min = 0xFFFFFFFF;
    if (header.e_phnum == 0) {
        printf("Issue: no ELF headers\n");
        return;
    }
    for (int i = 0; i < header.e_phnum; i++) {
        memcpy((char *)&pHeader, (char *)ELF_baseadr + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));
        printf("section: align: 0x%p vaddr->0x%p sizef->0x%p sizem->0x%p\n", pHeader.p_align, pHeader.p_vaddr, pHeader.p_filesz, pHeader.p_memsz);
        if (pHeader.p_type != 1) {
            continue;
        }
        if (pHeader.p_vaddr + pHeader.p_memsz > max) {
            max = pHeader.p_vaddr + pHeader.p_memsz;
        }
        if (pHeader.p_vaddr < min) {
            min = pHeader.p_vaddr;
        }
    }

    uint32_t pagecount = ((max - min) / 4096) + 5;

    multitasking::process_pagerange pageranges[PROCESS_MAX_PAGE_RANGES];
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        pageranges[i] = {0, 0, 0};
    }

    pageranges[0] = {(uint32_t)memalloc::page::phys_malloc(pagecount), min, pagecount};
    multitasking::setPageRange(pageranges);

    printf("---Program Headers---\n");
    for (int i = 0; i < header.e_phnum; i++) {
        memcpy((char *)&pHeader, (char *)ELF_baseadr + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));

        printf("section: align: 0x%p vaddr->0x%p sizef->0x%p sizem->0x%p\n", pHeader.p_align, pHeader.p_vaddr, pHeader.p_filesz, pHeader.p_memsz);
        if (pHeader.p_type != 1) {
            printf("ignoring section of type: 0x%p\n", pHeader.p_type);
            continue;
        }

        memset((uint8_t *)pHeader.p_vaddr, 0, pHeader.p_memsz);
        memcpy((char *)pHeader.p_vaddr, (char *)ELF_baseadr + pHeader.p_offset, pHeader.p_filesz);

        if (pHeader.p_vaddr + pHeader.p_memsz > max) {
            max = pHeader.p_vaddr + pHeader.p_memsz;
        }
        if (pHeader.p_vaddr < min) {
            min = pHeader.p_vaddr;
        }
    }
    printf("max: %u\n", max);
    printf("min: %u\n", min);
    printf("Inital program memory size: %u\n", max - min);

    printf("Entry point: 0x%p\n", header.e_entry);

    multitasking::create_task((void *)(max + (4096 * 4)), (void *)header.e_entry, pageranges);
    multitasking::unsetPageRange(pageranges);
}