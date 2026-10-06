#include <vix/arch/common/paging.h>
#include <vix/fs/vfs.h>
#include <vix/status.h>
#include <vix/abi/execfmt/elf.h>
#include <vix/debug.h>
#include <string.h>
#include <vix/mm/pmm.h>
#include <vix/mm/vmm.h>
#include <vix/macros.h>

status::StatusOr<void *> execfmt::elf::load_elf32(arch::vmm::pt_t pt, std::shared_ptr<struct vfs::vnode> elf_file) {
    // FIXME: BUG: I think one could load an ELF that loads in kernel space with this and REALLY fuck around!!
    struct elf_header header;
    struct elf_program_header pHeader;

    RUN_OR_RETURN(
        vfs::read(elf_file, 0, &header, sizeof(header)),
        {
            if (value != sizeof(header)) {
                return status::StatusCode::EGENERIC;
            }
        },
        {}
    );

    uint32_t max = 0;
    uint32_t min = 0xFFFFFFFF;
    if (header.e_phnum == 0) {
        DEBUG_PRINTF("Issue: no ELF headers\n");
        return status::StatusCode::EGENERIC;
    }
    for (int i = 0; i < header.e_phnum; i++) {
        RUN_OR_RETURN(
            vfs::read(elf_file, header.e_phoff + (header.e_phentsize * i), &pHeader, sizeof(pHeader)),
            {
                if (value != sizeof(pHeader)) {
                    return status::StatusCode::EGENERIC;
                }
            },
            {}
        );
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

    uint32_t max_v = ALIGN_UP(max, CONFIG_ARCH_PAGE_SIZE);
    uint32_t min_v = ALIGN_DOWN(min, CONFIG_ARCH_PAGE_SIZE);

    uint32_t pagecount = ((max_v - min_v) / CONFIG_ARCH_PAGE_SIZE);

    for (uint32_t i = 0; i < pagecount; i++) {
        mm::paddr_t allocated_phys;
        ASSIGN_OR_PANIC(allocated_phys, mm::pmm::alloc_contiguous(pagecount));
        arch::vmm::set_page_pt(
            pt,
            min_v + (i *CONFIG_ARCH_PAGE_SIZE),
            allocated_phys + (i *CONFIG_ARCH_PAGE_SIZE),
            arch::vmm::FLAGS_PRESENT | arch::vmm::FLAGS_USER
        );
    }

    // zero allocated memory
    memset((void *)min_v, 0, pagecount * CONFIG_ARCH_PAGE_SIZE);

    for (int i = 0; i < header.e_phnum; i++) {
        RUN_OR_RETURN(
            vfs::read(elf_file, header.e_phoff + (header.e_phentsize * i), &pHeader, sizeof(pHeader)),
            {
                if (value != sizeof(pHeader)) {
                    return status::StatusCode::EGENERIC;
                }
            },
            {}
        );

        if (!(pHeader.p_type == PT_LOAD)) {
            DEBUG_PRINTF("ignoring section of type: 0x%p\n", pHeader.p_type);
            continue;
        }

        memset((void *)pHeader.p_vaddr, 0, pHeader.p_memsz);
        RUN_OR_RETURN(
            vfs::read(elf_file, pHeader.p_offset, (void *)pHeader.p_vaddr, pHeader.p_filesz),
            {
                if (value != pHeader.p_filesz) {
                    return status::StatusCode::EGENERIC;
                }
            },
            {}
        );
    }

    DEBUG_PRINTF("min: 0x%p max: 0x%p entry: 0x%p\n", min, max, header.e_entry);

    return (void *)header.e_entry;
}
