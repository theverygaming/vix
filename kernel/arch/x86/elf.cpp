#include <vix/mm/mm.h>
#include <string.h>
#include <string>
#include <vector>
#include <vix/arch/common/cpu.h>
#include <vix/arch/elf.h>
#include <vix/arch/multitasking.h>
#include <vix/config.h>
#include <vix/debug.h>
#include <vix/mm/kheap.h>
#include <vix/mm/pmm.h>
#include <vix/status.h>

void elf::load_program(void *ELF_baseadr, std::vector<std::string> *argv, bool replace_task, int replace_pid, struct arch::full_ctx *regs) {
    ElfHeader header;
    ElfProgramHeader pHeader;

    memcpy(&header, ELF_baseadr, sizeof(ElfHeader));

    uint32_t max = 0;
    uint32_t min = 0xFFFFFFFF;
    if (header.e_phnum == 0) {
        DEBUG_PRINTF("Issue: no ELF headers\n");
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

    if (min_v % CONFIG_ARCH_PAGE_SIZE != 0) {
        min_v -= min_v % CONFIG_ARCH_PAGE_SIZE;
    }

    if (max_v % CONFIG_ARCH_PAGE_SIZE != 0) {
        max_v += CONFIG_ARCH_PAGE_SIZE - (max_v % CONFIG_ARCH_PAGE_SIZE);
    }

    uint32_t pagecount = ((max_v - min_v) / CONFIG_ARCH_PAGE_SIZE) + 41;

    std::vector<multitasking::process_pagerange> pageranges;

    mm::paddr_t allocated_phys;
    ASSIGN_OR_PANIC(allocated_phys, mm::pmm::alloc_contiguous(pagecount));
    pageranges.push_back({.phys_base = allocated_phys,
                          .virt_base = min_v,
                          .pages = pagecount,
                          .type = multitasking::process_pagerange::range_type::STATIC});
    ASSIGN_OR_PANIC(allocated_phys, mm::pmm::alloc_contiguous(1));
    pageranges.push_back({.phys_base = allocated_phys,
                          .virt_base = max_v + (CONFIG_ARCH_PAGE_SIZE * 47),
                          .pages = 1,
                          .type = multitasking::process_pagerange::range_type::BREAK});

    std::vector<multitasking::process_pagerange> old_pageranges;
    multitasking::createPageRange(&old_pageranges);
    multitasking::setPageRange(&pageranges);

    // zero all allocated memory
    for (size_t i = 0; i < pageranges.size(); i++) {
        memset((void *)pageranges[i].virt_base, 0, pageranges[i].pages * CONFIG_ARCH_PAGE_SIZE);
    }

    //struct multitasking::x86_process::tls_info tls;

    // DEBUG_PRINTF("---Program Headers---\n");
    for (int i = 0; i < header.e_phnum; i++) {
        memcpy(&pHeader, ((char *)ELF_baseadr) + header.e_phoff + (header.e_phentsize * i), sizeof(pHeader));

        // DEBUG_PRINTF("section: align: 0x%p vaddr->0x%p sizef->0x%p sizem->0x%p\n", pHeader.p_align, pHeader.p_vaddr, pHeader.p_filesz, pHeader.p_memsz);

        if (!(pHeader.p_type == 1 || pHeader.p_type == 7)) { // only load PT_LOAD and PT_TLS
            DEBUG_PRINTF("    ignoring section of type: 0x%p\n", pHeader.p_type);
            continue;
        }

        /*if (pHeader.p_type == 7) {        // PT_TLS
            if (tls.tlsdata == nullptr) { // can't have two of em (i think you can but not with this loader for now)
                DEBUG_PRINTF("    loaded TLS\n");
                tls.tls_size = pHeader.p_memsz;
                tls.tlsdata_size = pHeader.p_filesz;
                tls.tlsdata = mm::kmalloc(tls.tlsdata_size);
                memcpy(tls.tlsdata, ((char *)ELF_baseadr) + pHeader.p_offset, tls.tlsdata_size);
                continue;
            } else {
                KERNEL_PANIC("skill issue");
            }
        }*/

        memset((void *)pHeader.p_vaddr, 0, pHeader.p_memsz);
        memcpy((void *)pHeader.p_vaddr, ((char *)ELF_baseadr) + pHeader.p_offset, pHeader.p_filesz);
    }
    /*
    DEBUG_PRINTF("max: 0x%p\n", max);
    DEBUG_PRINTF("min: 0x%p\n", min);
    DEBUG_PRINTF("Initial program memory size: %u\n", max - min);

    DEBUG_PRINTF("Entry point: 0x%p\n", header.e_entry);
    */

    multitasking::unsetPageRange(&pageranges);
    multitasking::setPageRange(&old_pageranges);

    if (replace_task) {
        //multitasking::replace_task((void *)(max + (CONFIG_ARCH_PAGE_SIZE * 40)), (void *)header.e_entry, &pageranges, argv, tls, replace_pid, regs);
    } else {
        multitasking::create_task((void *)(max + (CONFIG_ARCH_PAGE_SIZE * 40)), (void *)header.e_entry, &pageranges, argv);
    }
}
