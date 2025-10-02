#include <string.h>
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
#include <vix/pgtable.h>

arch::vmm::pt_t arch::vmm::kernel_pt = {
    .level = 2,
    .ptr = 0,
};

extern "C" void loadPageDirectory(void *address);
extern "C" void reloadPageDirectory();
extern "C" void *getPageDirectory();
extern "C" void enablePaging();

static inline void invlpg(uintptr_t virtaddr) {
    asm volatile("invlpg (%0)" ::"r"(virtaddr) : "memory");
}

static mm::vaddr_t hhdm_offset(mm::paddr_t addr) {
    if (addr <= CONFIG_HHDM_PHYS_BASE ||
        addr >= (CONFIG_HHDM_PHYS_BASE + CONFIG_HHDM_SIZE)) {
        KERNEL_PANIC("hhdm_offset: out of HHDM range: 0x%p", addr);
    }
    return (addr - CONFIG_HHDM_PHYS_BASE) + CONFIG_HHDM_VIRT_BASE;
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

arch::pgtable::pdl4_t *arch::pgtable::kernel_global_pdl4 = nullptr;

void paging::init() {
    // TODO: we should ask the allocator to give us memory in the HHDM range
    size_t alloc_bytes = (
        // page directory
        (1024*4)
        // page tables
        + ((1024 - (CONFIG_KERNEL_HIGHER_HALF >> 22)) * (1024 * 4))
    );
    ASSIGN_OR_PANIC(arch::vmm::kernel_pt.ptr, mm::pmm::alloc_contiguous(
        alloc_bytes / CONFIG_ARCH_PAGE_SIZE
    ));
    // initialize the page directory
    uint32_t *pd = (uint32_t*)hhdm_offset(arch::vmm::kernel_pt.ptr);
    memset(pd, 0, alloc_bytes);
    // create and initialize page tables for the kernel address space
    uint32_t (*pt)[1024] = (uint32_t(*)[1024])(arch::vmm::kernel_pt.ptr + CONFIG_ARCH_PAGE_SIZE);
    for(unsigned int i = 0; i < (1024 - (CONFIG_KERNEL_HIGHER_HALF >> 22)); i++) {
        pd[i + (CONFIG_KERNEL_HIGHER_HALF >> 22)] = (uintptr_t)pt[i] & 0xFFFFF000;
        pd[i + (CONFIG_KERNEL_HIGHER_HALF >> 22)] |= 1; // present
        pd[i + (CONFIG_KERNEL_HIGHER_HALF >> 22)] |= (1 << 1); // RW
    }
    // map the HHDM (only thing that was mapped before too)
    for (uintptr_t addr = CONFIG_HHDM_VIRT_BASE; addr < CONFIG_HHDM_VIRT_BASE+CONFIG_HHDM_SIZE; addr += CONFIG_ARCH_PAGE_SIZE) {
        arch::vmm::set_page(
            addr,
            CONFIG_HHDM_PHYS_BASE + (addr - CONFIG_HHDM_VIRT_BASE),
            arch::vmm::FLAGS_PRESENT
        );
    }
    arch::vmm::load_pt(arch::vmm::kernel_pt);

    arch::pgtable::kernel_global_pdl4 = (arch::pgtable::pdl4_t *)arch::pgtable::hhdm_to_virt(arch::vmm::kernel_pt.ptr);
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
arch::vmm::set_page_pt(pt_t pt, uintptr_t virt, uintptr_t phys, unsigned int flags) {
    pte_t pte;
    ASSIGN_OR_PANIC(pte, walk(pt, virt));
    auto pter = read_pte(pte);

    unsigned int flags_old = pter.first;

    write_pte(pte, phys, flags);

    return flags_old;
}

unsigned int
arch::vmm::set_page(uintptr_t virt, uintptr_t phys, unsigned int flags) {
    return set_page_pt(kernel_pt, virt, phys, flags);
}

status::StatusOr<arch::vmm::pt_t> arch::vmm::alloc_pt(short level) {
    if (level != 2) {
        KERNEL_PANIC("not implemented: alloc_pt level != 2");
    }
    arch::vmm::pt_t pd = {
        .level = 2,
        .ptr = 0,
    };
    // TODO: we should ask the allocator to give us memory in the HHDM range
    ASSIGN_OR_PANIC(pd.ptr, mm::pmm::alloc_contiguous(
        (
            // page directory
            (1024*4)
        ) / CONFIG_ARCH_PAGE_SIZE
    ));
    uint32_t *pd_v = (uint32_t*)hhdm_offset(pd.ptr);
    memset(pd_v, 0, 1024 * 4);

    // walk still can't create new page tables on the fly, so we just allocate a bunch here..
    // BUG: memory leak definitely here lmao
    for(unsigned int i = 0; i < (CONFIG_KERNEL_HIGHER_HALF >> 22); i++) {
        mm::paddr_t pt;
        // TODO: we should ask the allocator to give us memory in the HHDM range
        ASSIGN_OR_PANIC(pt, mm::pmm::alloc_contiguous(
            (
                // page table
                (1024*4)
            ) / CONFIG_ARCH_PAGE_SIZE
        ));

        pd_v[i] = pt & 0xFFFFF000;
        pd_v[i] |= 1; // present
        pd_v[i] |= (1 << 1); // RW
        pd_v[i] |= (1 << 2); // user

        uint32_t *pt_v = (uint32_t*)hhdm_offset(pt);
        for(int j = 0; j < 1024; j++) {
            pt_v[j] = 0;
        }
    }

    // for the kernel space, copy entries from the kernel page directory
    uint32_t *k_pd_v = (uint32_t*)hhdm_offset(arch::vmm::kernel_pt.ptr);
    for(unsigned int i = 0; i < (1024 - (CONFIG_KERNEL_HIGHER_HALF >> 22)); i++) {
        unsigned int pd_idx = i + (CONFIG_KERNEL_HIGHER_HALF >> 22);
        pd_v[pd_idx] = k_pd_v[pd_idx];
    }

    return pd;
}

void arch::vmm::free_pt(arch::vmm::pt_t pt) {
    mm::pmm::free_contiguous(
        pt.ptr,
        (
            // page directory
            (1024*4)
        ) / CONFIG_ARCH_PAGE_SIZE
    );
}

arch::vmm::pt_t arch::vmm::get_active_pt() {
    return {
        .level = 2,
        .ptr = (mm::paddr_t)getPageDirectory(),
    };
}

void arch::vmm::load_pt(pt_t pt) {
    if (pt.level != 2) {
        KERNEL_PANIC("load_pt invalid level");
    }
    loadPageDirectory((void *)pt.ptr);
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
    mm::paddr_t pt_paddr = (mm::paddr_t)pt.ptr;
    for (int level = CONFIG_ARCH_PAGING_LEVELS; level >= 1; level--) {
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
        return status::StatusCode::EGENERIC;
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
    arch::vmm::load_pt(arch::vmm::get_active_pt());
}
