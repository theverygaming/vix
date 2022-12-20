#include <arch/elf.h>
#include <arch/generic/memory.h>
#include <arch/multitasking.h>
#include <cppstd/string.h>
#include <cppstd/vector.h>
#include <log.h>
#include <memory_alloc/memalloc.h>
#include <stdlib.h>

void elf::load_program(void *ELF_baseadr, std::vector<std::string> *argv, bool replace_task, int replace_pid, isr::registers *regs) {
    ElfHeader header;
    ElfProgramHeader pHeader;

    memcpy(&header, ELF_baseadr, sizeof(ElfHeader));

    uint32_t max = 0;
    uint32_t min = 0xFFFFFFFF;
    if (header.e_phnum == 0) {
        LOG_DEBUG("Issue: no ELF headers\n");
        return;
    }
    for (int i = 0; i < header.e_phnum; i++) {
        memcpy(&pHeader, ((char *)ELF_baseadr) + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));
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

    uint32_t max_v = max;
    uint32_t min_v = min;

    if (min_v % ARCH_PAGE_SIZE != 0) {
        min_v -= min_v % ARCH_PAGE_SIZE;
    }

    if (max_v % ARCH_PAGE_SIZE != 0) {
        max_v += ARCH_PAGE_SIZE - (max_v % ARCH_PAGE_SIZE);
    }

    uint32_t pagecount = ((max_v - min_v) / ARCH_PAGE_SIZE) + 5;

    std::vector<multitasking::process_pagerange> pageranges;

    pageranges.push_back({.phys_base = (uint32_t)memalloc::page::phys_malloc(pagecount), .virt_base = min_v, .pages = pagecount, .type = multitasking::process_pagerange::range_type::STATIC});
    pageranges.push_back({.phys_base = (uint32_t)memalloc::page::phys_malloc(1), .virt_base = max_v + (ARCH_PAGE_SIZE * 6), .pages = 1, .type = multitasking::process_pagerange::range_type::BREAK});

    std::vector<multitasking::process_pagerange> old_pageranges;
    multitasking::createPageRange(&old_pageranges);
    multitasking::setPageRange(&pageranges);

    DEBUG_PRINTF("---Program Headers---\n");
    for (int i = 0; i < header.e_phnum; i++) {
        memcpy(&pHeader, ((char *)ELF_baseadr) + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));

        DEBUG_PRINTF("section: align: 0x%p vaddr->0x%p sizef->0x%p sizem->0x%p\n", pHeader.p_align, pHeader.p_vaddr, pHeader.p_filesz, pHeader.p_memsz);
        if (pHeader.p_type != 1) {
            DEBUG_PRINTF("ignoring section of type: 0x%p\n", pHeader.p_type);
            continue;
        }

        memset((void *)pHeader.p_vaddr, 0, pHeader.p_memsz);
        memcpy((void *)pHeader.p_vaddr, ((char *)ELF_baseadr) + pHeader.p_offset, pHeader.p_filesz);
    }
    DEBUG_PRINTF("max: %u\n", max);
    DEBUG_PRINTF("min: %u\n", min);
    DEBUG_PRINTF("Inital program memory size: %u\n", max - min);

    DEBUG_PRINTF("Entry point: 0x%p\n", header.e_entry);

    multitasking::unsetPageRange(&pageranges);
    multitasking::setPageRange(&old_pageranges);

    if (replace_task) {
        multitasking::replace_task((void *)(max + (ARCH_PAGE_SIZE * 4)), (void *)header.e_entry, &pageranges, argv, replace_pid, regs);
    } else {
        multitasking::create_task((void *)(max + (ARCH_PAGE_SIZE * 4)), (void *)header.e_entry, &pageranges, argv);
    }
}
