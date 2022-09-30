#include <arch/x86/elf.h>
#include <arch/x86/generic/memory.h>
#include <arch/x86/multitasking.h>
#include <cppstd/vector.h>
#include <log.h>
#include <memory_alloc/memalloc.h>
#include <stdlib.h>

void elf::load_program(void *ELF_baseadr, std::vector<char *> *argv, bool replace_task, int replace_pid) {
    ElfHeader header;
    ElfProgramHeader pHeader;

    stdlib::memcpy(&header, ELF_baseadr, sizeof(ElfHeader));

    uint32_t max = 0;
    uint32_t min = 0xFFFFFFFF;
    if (header.e_phnum == 0) {
        LOG_DEBUG("Issue: no ELF headers\n");
        return;
    }
    for (int i = 0; i < header.e_phnum; i++) {
        stdlib::memcpy(&pHeader, ((char *)ELF_baseadr) + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));
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

    uint32_t pagecount = ((max - min) / ARCH_PAGE_SIZE) + 5;

    multitasking::process_pagerange pageranges[PROCESS_MAX_PAGE_RANGES];
    for (int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
        pageranges[i] = {0, 0, 0};
    }

    pageranges[0] = {(uint32_t)memalloc::page::phys_malloc(pagecount), min, pagecount};
    multitasking::process_pagerange old_pageranges[PROCESS_MAX_PAGE_RANGES];
    multitasking::createPageRange(old_pageranges);
    multitasking::setPageRange(pageranges);

    DEBUG_PRINTF("---Program Headers---\n");
    for (int i = 0; i < header.e_phnum; i++) {
        stdlib::memcpy(&pHeader, ((char *)ELF_baseadr) + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));

        DEBUG_PRINTF("section: align: 0x%p vaddr->0x%p sizef->0x%p sizem->0x%p\n", pHeader.p_align, pHeader.p_vaddr, pHeader.p_filesz, pHeader.p_memsz);
        if (pHeader.p_type != 1) {
            DEBUG_PRINTF("ignoring section of type: 0x%p\n", pHeader.p_type);
            continue;
        }

        stdlib::memset((void *)pHeader.p_vaddr, 0, pHeader.p_memsz);
        stdlib::memcpy((void *)pHeader.p_vaddr, ((char *)ELF_baseadr) + pHeader.p_offset, pHeader.p_filesz);

        if (pHeader.p_vaddr + pHeader.p_memsz > max) {
            max = pHeader.p_vaddr + pHeader.p_memsz;
        }
        if (pHeader.p_vaddr < min) {
            min = pHeader.p_vaddr;
        }
    }
    DEBUG_PRINTF("max: %u\n", max);
    DEBUG_PRINTF("min: %u\n", min);
    DEBUG_PRINTF("Inital program memory size: %u\n", max - min);

    DEBUG_PRINTF("Entry point: 0x%p\n", header.e_entry);

    multitasking::unsetPageRange(pageranges);
    multitasking::setPageRange(old_pageranges);

    if (replace_task) {
        multitasking::replace_task((void *)(max + (ARCH_PAGE_SIZE * 4)), (void *)header.e_entry, pageranges, argv, replace_pid);
    } else {
        multitasking::create_task((void *)(max + (ARCH_PAGE_SIZE * 4)), (void *)header.e_entry, pageranges, argv);
    }
}
