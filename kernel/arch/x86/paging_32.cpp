#include <utility>
#include <vix/kprintf.h>
#include <vix/status.h>
#include <string.h>
#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/arch/paging.h>
#include <vix/config.h>
#include <vix/mm/mm.h>
#include <vix/panic.h>
#include <vix/stdio.h>

uint32_t (*pagetables)[1024] = (uint32_t (*)[1024])(KERNEL_VIRT_ADDRESS + PAGE_TABLES_OFFSET);
uint32_t *page_directory = (uint32_t *)(KERNEL_VIRT_ADDRESS + PAGE_DIRECTORY_OFFSET);

arch::vmm::pt_t arch::vmm::kernel_pt = (uintptr_t)KERNEL_PHYS_ADDRESS + PAGE_DIRECTORY_OFFSET;

extern "C" void loadPageDirectory(void *address);
extern "C" void reloadPageDirectory();
extern "C" void enablePaging();

static inline void invlpg(void *virtaddr) {
    uint32_t _virtaddr = (uint32_t)virtaddr;
    asm volatile("invlpg (%0)" ::"r"(_virtaddr) : "memory");
}

void paging::init() {
    clearPageTables((void *)0x0, KERNEL_VIRT_ADDRESS / CONFIG_ARCH_PAGE_SIZE);
}

void *paging::get_physaddr(void *virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    // TODO: check if entry is actually present
    return (void *)(pagetables[pdindex][ptindex] & 0xFFFFF000);
}

void *paging::get_physaddr_unaligned(void *virtualaddr) {
    uint64_t misalignment = ((uint64_t)virtualaddr) % CONFIG_ARCH_PAGE_SIZE;
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    // TODO: check if entry is actually present
    return (void *)((pagetables[pdindex][ptindex] & 0xFFFFF000) + misalignment);
}

void paging::clearPageTables(
    void *virtualaddr, uint32_t pagecount, bool massflush
) {
    uint32_t pDirIndex;
    uint32_t pTableIndex;
    bool do_invlpg;

    for (size_t i = 0; i < pagecount; i++) {
        pDirIndex = (uint32_t)virtualaddr >> 22;
        pTableIndex = (uint32_t)virtualaddr >> 12 & 0x03FF;
        do_invlpg = (pagetables[pDirIndex][pTableIndex] & 0x1) && !massflush;

        pagetables[pDirIndex][pTableIndex] = 0;
        if (do_invlpg) {
            invlpg(virtualaddr);
        }
        virtualaddr = ((uint8_t *)virtualaddr) + 4096;
    }

    if (massflush) {
        reloadPageDirectory();
    }
}

bool paging::is_readable(const void *virtualaddr) {
    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;
    return pagetables[pdindex][ptindex] & 0x1;
}

uintptr_t arch::vmm::get_page(uintptr_t virt, unsigned int *flags) {
    pte_t pte;
    ASSIGN_OR_PANIC(pte, walk(kernel_pt, virt));
    auto pter = read_pte(pte);

    *flags = pter.first;
    return (uintptr_t)pter.second;
}

unsigned int
arch::vmm::set_page(uintptr_t virt, uintptr_t phys, unsigned int flags) {
    pte_t pte;
    ASSIGN_OR_PANIC(pte, walk(kernel_pt, virt));
    auto pter = read_pte(pte);

    unsigned int flags_old = pter.first;

    write_pte(pte, phys, flags);

    return flags_old;
}

status::StatusOr<arch::vmm::pt_t> arch::vmm::alloc_user_pt() {
    KERNEL_PANIC("not implemented");
}

void arch::vmm::free_user_pt(arch::vmm::pt_t pt) {
    KERNEL_PANIC("not implemented");
}

static uint32_t phys_read(mm::paddr_t addr) {
    // currently exclusively works for the page tables :3, will explode for everything else
    return *((uint32_t *)(addr + (KERNEL_VIRT_ADDRESS - KERNEL_PHYS_ADDRESS)));
}

static void phys_write(mm::paddr_t addr, uint32_t val) {
    // currently exclusively works for the page tables :3, will explode for everything else
    *((uint32_t *)(addr + (KERNEL_VIRT_ADDRESS - KERNEL_PHYS_ADDRESS))) = val;
}

status::StatusOr<arch::vmm::pte_t> arch::vmm::walk(
    arch::vmm::pt_t pt,
    mm::vaddr_t vaddr,
    bool create_pts,
    unsigned int create_pts_flags
) {
    if (create_pts) {
        KERNEL_PANIC("create_pts not implemented");
    }
    // inspired by xv6-riscv :3
    mm::paddr_t pt_paddr = (mm::paddr_t)pt;
    for (int level = 2; level >= 1; level--) {
        // index into page table
        pt_paddr += ((vaddr >> (2 + level * 10)) & 0x03FF) * 4;
        // on the final level, there's no need to look any deeper, if we look deeper we'll get the physical address!
        if (level == 1) {
            break;
        }
        uint32_t pte_val = phys_read(pt_paddr);
        // present bit set?
        if (pte_val & 0x1) {
            pt_paddr = pte_val & 0xFFFFF000;
            continue;
        }
        return status::StatusCode::UNKNOWN_ERR;
    }
    return (arch::vmm::pte_t)pt_paddr;
}

std::pair<unsigned int, mm::paddr_t> arch::vmm::read_pte(arch::vmm::pte_t pte) {
    uint32_t pte_val = phys_read(pte);

    unsigned int flags = 0;
    if (pte_val & 0x1) {
        flags |= arch::vmm::FLAGS_PRESENT;
    }
    if ((pte_val >> 5) & 0x1) {
        flags |= arch::vmm::FLAGS_ACCESSED;
    }
    if ((pte_val >> 6) & 0x1) {
        flags |= arch::vmm::FLAGS_DIRTY;
    }
    if ((pte_val >> 4) & 0x1) {
        flags |= arch::vmm::FLAGS_CACHE_DISABLE;
    }
    if ((pte_val >> 3) & 0x1) {
        flags |= arch::vmm::FLAGS_WRITE_THROUGH;
    } else {
        flags |= arch::vmm::FLAGS_WRITE_BACK;
    }
    if ((pte_val >> 2) & 0x1) {
        flags |= arch::vmm::FLAGS_USER;
    }
    if (((pte_val >> 1) & 0x1) == 0) {
        flags |= arch::vmm::FLAGS_READ_ONLY;
    }
    if ((pte_val >> 8) & 0x1) {
        flags |= arch::vmm::FLAGS_NO_FLUSH_ON_PRIV_CHANGE;
    }
    if ((pte_val >> 9) & 0x1) {
        flags |= arch::vmm::FLAGS_OS_FLAG_1;
    }
    if ((pte_val >> 10) & 0x1) {
        flags |= arch::vmm::FLAGS_OS_FLAG_2;
    }

    return {
        flags,
        (mm::paddr_t)(pte_val & 0xFFFFF000),
    };
}

void arch::vmm::write_pte(
    arch::vmm::pte_t pte, mm::paddr_t phys, unsigned int flags
) {
    uint32_t pte_val = phys & 0xFFFFF000;

    if (flags & arch::vmm::FLAGS_PRESENT) {
        pte_val |= 0x1;
    }
    if (flags & arch::vmm::FLAGS_ACCESSED) {
        pte_val |= (0x1 << 5);
    }
    if (flags & arch::vmm::FLAGS_DIRTY) {
        pte_val |= (0x1 << 6);
    }
    if (flags & arch::vmm::FLAGS_CACHE_DISABLE) {
        pte_val |= (0x1 << 4);
    }
    if (flags & arch::vmm::FLAGS_WRITE_THROUGH) {
        pte_val |= (0x1 << 3);
    }
    if (flags & arch::vmm::FLAGS_USER) {
        pte_val |= (0x1 << 2);
    }
    if ((flags & arch::vmm::FLAGS_READ_ONLY) == 0) {
        pte_val |= (0x1 << 1);
    }
    if (flags & arch::vmm::FLAGS_NO_FLUSH_ON_PRIV_CHANGE) {
        pte_val |= (0x1 << 8);
    }
    if (flags & arch::vmm::FLAGS_OS_FLAG_1) {
        pte_val |= (0x1 << 9);
    }
    if (flags & arch::vmm::FLAGS_OS_FLAG_2) {
        pte_val |= (0x1 << 10);
    }

    phys_write(pte, pte_val);
}

void arch::vmm::flush_tlb_single(uintptr_t virt) {
    invlpg((void *)virt);
}

void arch::vmm::flush_tlb_all() {
    reloadPageDirectory();
}
