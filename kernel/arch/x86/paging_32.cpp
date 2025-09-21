#include <utility>
#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/arch/paging.h>
#include <vix/config.h>
#include <vix/kprintf.h>
#include <vix/mm/mm.h>
#include <vix/mm/pmm.h>
#include <vix/panic.h>
#include <vix/status.h>
#include <vix/stdio.h>

arch::vmm::pt_t arch::vmm::kernel_pt = 0;

extern "C" void loadPageDirectory(void *address);
extern "C" void reloadPageDirectory();
extern "C" void enablePaging();

static inline void invlpg(uintptr_t virtaddr) {
    asm volatile("invlpg (%0)" ::"r"(virtaddr) : "memory");
}

void paging::init() {
    // TODO: we should ask the allocator to give us memory in the HHDM range
    ASSIGN_OR_PANIC(arch::vmm::kernel_pt, mm::pmm::alloc_contiguous(
        (
            // page directory
            (1024*4)
            // page tables
            + (1024*(1024*4))
        ) / CONFIG_ARCH_PAGE_SIZE
    ));
    // initialize the page directory
    uint32_t *pd = (uint32_t*)arch::vmm::kernel_pt;
    uint32_t (*pt)[1024] = (uint32_t(*)[1024])(arch::vmm::kernel_pt + CONFIG_ARCH_PAGE_SIZE);
    for(int i = 0; i < 1024; i++) {
        pd[i] = (uintptr_t)pt[i] & 0xFFFFF000;
        pd[i] |= 1; // present
        pd[i] |= (1 << 1); // RW
        pd[i] |= (1 << 1); // RW
        for(int j = 0; j < 1024; j++) {
            pt[i][j] = 0;
        }
    }
    // map the HHDM (only thing that was mapped before too)
    for (uintptr_t addr = CONFIG_HHDM_VIRT_BASE; addr < CONFIG_HHDM_VIRT_BASE+CONFIG_HHDM_SIZE; addr += CONFIG_ARCH_PAGE_SIZE) {
        arch::vmm::set_page(
            addr,
            CONFIG_HHDM_PHYS_BASE + (addr - CONFIG_HHDM_VIRT_BASE),
            arch::vmm::FLAGS_PRESENT
        );
    }
    loadPageDirectory(pd);
}

uintptr_t arch::vmm::get_page(uintptr_t virt, unsigned int *flags) {
    pte_t pte;
    ASSIGN_OR_PANIC(pte, walk(kernel_pt, virt));
    auto pter = read_pte(pte);

    if (flags != nullptr) {
        *flags = pter.first;
    }
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
    if (addr <= CONFIG_HHDM_PHYS_BASE ||
        addr >= (CONFIG_HHDM_PHYS_BASE + CONFIG_HHDM_SIZE)) {
        KERNEL_PANIC("phys_read: out of HHDM range: 0x%p", addr);
    }
    return *((uint32_t *)((addr - CONFIG_HHDM_PHYS_BASE) + CONFIG_HHDM_VIRT_BASE));
}

static void phys_write(mm::paddr_t addr, uint32_t val) {
    if (addr <= CONFIG_HHDM_PHYS_BASE ||
        addr >= (CONFIG_HHDM_PHYS_BASE + CONFIG_HHDM_SIZE)) {
        KERNEL_PANIC("phys_read: out of HHDM range: 0x%p", addr);
    }
    *((uint32_t *)((addr - CONFIG_HHDM_PHYS_BASE) + CONFIG_HHDM_VIRT_BASE)) = val;
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
    invlpg(virt);
}

void arch::vmm::flush_tlb_all() {
    reloadPageDirectory();
}
