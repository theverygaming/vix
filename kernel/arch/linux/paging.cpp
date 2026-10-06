#include <vix/debug.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/mm.h>
#include <vix/mm/pmm.h>
#include <vix/panic.h>
#include <vix/config.h>
#include <vix/arch/common/paging.h>
#include <vix/arch/linux-syscalls.h>
#include <vix/arch/paging.h>
#include <string.h>

// TODO: we currently only implement set_page and get_page for the kernel MM to work. Userspace stuff is still TODO!

extern int memfd;
extern size_t memfd_bytes;

static int proc_maps_fd;

arch::vmm::pt_t arch::vmm::kernel_pt = {
    .level = 2,
    .ptr = 0,
};

typedef uint32_t pt_entry_t;

static void paddr_into_pte(pt_entry_t *e, mm::paddr_t paddr) {
    *e = (*e & 0xFFF) | (paddr & 0xFFFFF000);
}

static void flags_into_pte(pt_entry_t *e, unsigned int flags) {
    // TODO: this doesn't fit all the flags lol
    *e = (*e & 0xFFFFF000) | (flags & 0xFFF);
}

static mm::paddr_t paddr_from_pte(pt_entry_t *e) {
    return *e & 0xFFFFF000; 
}

static unsigned int flags_from_pte(pt_entry_t *e) {
    // TODO: this doesn't fit all the flags lol
    return *e & 0xFFF;
}

static mm::vaddr_t hhdm_offset(mm::paddr_t addr) {
    if (addr <= CONFIG_HHDM_PHYS_BASE ||
        addr >= (CONFIG_HHDM_PHYS_BASE + CONFIG_HHDM_SIZE)) {
        KERNEL_PANIC("hhdm_offset: out of HHDM range: 0x%p", addr);
    }
    return (addr - CONFIG_HHDM_PHYS_BASE) + CONFIG_HHDM_VIRT_BASE;
}

static pt_entry_t phys_pt_read(mm::paddr_t addr) {
    return *((pt_entry_t *)hhdm_offset(addr));
}

static void phys_pt_write(mm::paddr_t addr, pt_entry_t val) {
    *((pt_entry_t *)hhdm_offset(addr)) = val;
}

static unsigned int pmq_get_flags(struct linux_procmap_query *q) {
    unsigned int flags = arch::vmm::FLAGS_PRESENT;
    if ((q->vma_flags & LINUX_PROCMAP_QUERY_VMA_READABLE) != 0 && (q->vma_flags & LINUX_PROCMAP_QUERY_VMA_WRITABLE) == 0) {
        flags |= arch::vmm::FLAGS_READ_ONLY;
    }
    if ((q->vma_flags & LINUX_PROCMAP_QUERY_VMA_EXECUTABLE) == 0) {
        flags |= arch::vmm::FLAGS_NO_EXECUTE;
    }
    return flags;
}

static void do_mmap(mm::vaddr_t vaddr, mm::paddr_t paddr, size_t n_pages, unsigned int flags) {
    int prot = LINUX_PROT_READ;
    if ((flags & arch::vmm::FLAGS_READ_ONLY) == 0) {
        prot |= LINUX_PROT_WRITE;
    }
    if ((flags & arch::vmm::FLAGS_NO_EXECUTE) == 0) {
        prot |= LINUX_PROT_EXEC;
    }
    void *virt_tgt = (void *)vaddr;
    void *mmap_res = linux_mmap(virt_tgt, n_pages * CONFIG_ARCH_PAGE_SIZE, prot, LINUX_MAP_SHARED | LINUX_MAP_FIXED, memfd, paddr);
    if (mmap_res != virt_tgt) {
        KERNEL_PANIC("linux mmap failed res: 0x%p", mmap_res);
    }
}

static void do_munmap(mm::vaddr_t vaddr, size_t n_pages) {
    int err = linux_munmap((void *)vaddr, n_pages * CONFIG_ARCH_PAGE_SIZE);
    if (err != 0) {
        KERNEL_PANIC("linux munmap failed with %d", err);
    }
}

void paging_init() {
    proc_maps_fd = linux_open("/proc/self/maps", LINUX_O_RDONLY, 0);
    if (proc_maps_fd < 0) {
        KERNEL_PANIC("got error %d", proc_maps_fd);
    }
    size_t alloc_bytes = (
        // page directory
        (1024 * sizeof(pt_entry_t))
        // kernel page tables
        + ((1024 - (CONFIG_KERNEL_HIGHER_HALF >> 22)) * (1024 * sizeof(pt_entry_t)))
    );
    DEBUG_PRINTF("need %u bytes for kernel PD & PTs\n", alloc_bytes);
    // TODO: we should ask the allocator to give us memory in the HHDM range
    ASSIGN_OR_PANIC(arch::vmm::kernel_pt.ptr, mm::pmm::alloc_contiguous(
        alloc_bytes / CONFIG_ARCH_PAGE_SIZE
    ));
    // initialize the page directory
    pt_entry_t *pd = (pt_entry_t*)hhdm_offset(arch::vmm::kernel_pt.ptr);
    memset(pd, 0, alloc_bytes);
    // create and initialize page tables for the kernel address space
    pt_entry_t (*pt)[1024] = (pt_entry_t(*)[1024])(arch::vmm::kernel_pt.ptr + CONFIG_ARCH_PAGE_SIZE);
    for(unsigned int i = 0; i < (1024 - (CONFIG_KERNEL_HIGHER_HALF >> 22)); i++) {
        paddr_into_pte(&pd[i + (CONFIG_KERNEL_HIGHER_HALF >> 22)], (uintptr_t)pt[i]);
        flags_into_pte(&pd[i + (CONFIG_KERNEL_HIGHER_HALF >> 22)], arch::vmm::FLAGS_PRESENT);
    }

    // add existing mappings into the page table
    mm::vaddr_t query_addr = 0;
    while (true) {
        struct linux_procmap_query query = {
            .size = sizeof(struct linux_procmap_query),
            .query_flags = LINUX_PROCMAP_QUERY_COVERING_OR_NEXT_VMA, // in
            .query_addr = query_addr, // in
            .vma_start = 0,
            .vma_end = 0,
            .vma_flags = 0,
            .vma_page_size = 0,
            .vma_offset = 0,
            .inode = 0,
            .dev_major = 0,
            .dev_minor = 0,
            .vma_name_size = 0, // in/out
            .build_id_size = 0, // in/out
            .vma_name_addr = 0, // in
            .build_id_addr = 0, // in
        };
        if (linux_ioctl(proc_maps_fd, LINUX_PROCMAP_QUERY, (unsigned long)&query) < 0) {
            // unmapped
            break;
        }
        query_addr = query.vma_end;
        // this is a 32-bit household!!! (these "page tables" don't even support 64-bit addresses so lmao) (this also simply avoids bullshit like the damn vsyscall mapping I can't seem to get rid of and like who needs 64-bit address space anyway like come on totally overrated!!)
        if (query.vma_start > 0xFFFFFFFF) {
            continue;
        }
        if (query.inode == 0) {
            KERNEL_PANIC("no backing file");
        }
        DEBUG_PRINTF("map: 0x%p-0x%p offset 0x%p\n", query.vma_start, query.vma_end, query.vma_offset);
        unsigned int flags = pmq_get_flags(&query);
        for (size_t i = query.vma_start; i < query.vma_end; i += CONFIG_ARCH_PAGE_SIZE) {
            arch::vmm::set_page(
                i,
                query.vma_offset + (i - query.vma_start),
                flags
            );
        }
    }

    arch::vmm::load_pt(arch::vmm::kernel_pt);
}

uintptr_t arch::vmm::get_page(uintptr_t virt, unsigned int *flags) {
    return get_page_pt(kernel_pt, virt, flags);
}

uintptr_t arch::vmm::get_page_pt(pt_t pt, uintptr_t virt, unsigned int *flags) {
    pte_t pte;
    ASSIGN_OR_PANIC(pte, walk(pt, virt));
    auto pter = read_pte(pte);

    if (flags != nullptr) {
        *flags = pter.first;
    }
    return (uintptr_t)pter.second;
}

unsigned int arch::vmm::set_page_pt(pt_t pt, uintptr_t virt, uintptr_t phys, unsigned int flags) {
    pte_t pte;
    ASSIGN_OR_PANIC(pte, walk(pt, virt));
    auto pter = read_pte(pte);

    unsigned int flags_old = pter.first;

    write_pte(pte, phys, flags);

    if (flags & arch::vmm::FLAGS_PRESENT) {
        do_mmap(virt, phys, 1, flags);
    } else if (flags_old & arch::vmm::FLAGS_PRESENT) {
        do_munmap(virt, 1);
    }

    return flags_old;
}

unsigned int arch::vmm::set_page(uintptr_t virt, uintptr_t phys, unsigned int flags) {
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
            (1024*sizeof(pt_entry_t))
        ) / CONFIG_ARCH_PAGE_SIZE
    ));
    pt_entry_t *pd_v = (pt_entry_t*)hhdm_offset(pd.ptr);
    memset(pd_v, 0, 1024 * 4);

    // walk still can't create new page tables on the fly, so we just allocate a bunch here..
    // BUG: memory leak definitely here lmao
    for(unsigned int i = 0; i < (CONFIG_KERNEL_HIGHER_HALF >> 22); i++) {
        mm::paddr_t pt;
        // TODO: we should ask the allocator to give us memory in the HHDM range
        ASSIGN_OR_PANIC(pt, mm::pmm::alloc_contiguous(
            (
                // page table
                (1024*sizeof(pt_entry_t))
            ) / CONFIG_ARCH_PAGE_SIZE
        ));

        paddr_into_pte(&pd_v[i], pt);
        flags_into_pte(&pd_v[i], arch::vmm::FLAGS_PRESENT);

        pt_entry_t *pt_v = (pt_entry_t*)hhdm_offset(pt);
        for(int j = 0; j < 1024; j++) {
            pt_v[j] = 0;
        }
    }

    // for the kernel space, copy entries from the kernel page directory
    pt_entry_t *k_pd_v = (pt_entry_t*)hhdm_offset(arch::vmm::kernel_pt.ptr);
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
            (1024*sizeof(pt_entry_t))
        ) / CONFIG_ARCH_PAGE_SIZE
    );
}

static arch::vmm::pt_t active_pt = {
    .level = 2,
    .ptr = 0,
};

arch::vmm::pt_t arch::vmm::get_active_pt() {
    return {
        .level = 2,
        .ptr = active_pt.ptr,
    };
}

static void swap_pt(arch::vmm::pt_t o, arch::vmm::pt_t n) {
    // over PDs
    for (size_t pdi = 0; pdi < 1024; pdi++) {
        mm::vaddr_t pd_vaddr = pdi << 22;
        size_t pd_offs = pdi * sizeof(pt_entry_t);
        pt_entry_t pdo = phys_pt_read(o.ptr + pd_offs);
        pt_entry_t pdn = phys_pt_read(o.ptr + pd_offs);
        bool old_pde_present = (flags_from_pte(&pdo) & arch::vmm::FLAGS_PRESENT) != 0;
        bool new_pde_present = (flags_from_pte(&pdo) & arch::vmm::FLAGS_PRESENT) != 0;
        // exact same or both non-present? We are done with this one :3
        if (pdo == pdn || (!old_pde_present && !new_pde_present)) {
            continue;
        }
        // quick path: old present, new non-present: unmap all old
        if (old_pde_present && !new_pde_present) {
            do_munmap(pd_vaddr, 1024);
            continue;
        }
        // over PTs
        for (size_t pti = 0; pti < 1024; pti++) {
            mm::vaddr_t pt_vaddr = pd_vaddr | pti << 12;
            size_t pt_offs = pti * sizeof(pt_entry_t);
            pt_entry_t pto = old_pde_present ? phys_pt_read(paddr_from_pte(&pdo) + pt_offs) : 0;
            pt_entry_t ptn = new_pde_present ? phys_pt_read(paddr_from_pte(&pdn) + pt_offs) : 0;
            bool old_pte_present = old_pde_present && (flags_from_pte(&pto) & arch::vmm::FLAGS_PRESENT) != 0;
            bool new_pte_present = new_pde_present && (flags_from_pte(&pto) & arch::vmm::FLAGS_PRESENT) != 0;
            // exact same or both non-present? We are done with this one :3
            if ((old_pte_present && new_pte_present && (pto == ptn)) || (!old_pte_present && !new_pte_present)) {
                continue;
            }
            // old present, new non-present: unmap
            if (old_pte_present && !new_pte_present) {
                do_munmap(pt_vaddr, 1);
                continue;
            }
            // old present or not, if the new one is present we just overwrite the mapping
            do_mmap(pt_vaddr, paddr_from_pte(&ptn), 1, flags_from_pte(&ptn));
        }
    }
}

void arch::vmm::load_pt(pt_t pt) {
    if (pt.level != 2) {
        KERNEL_PANIC("load_pt invalid level");
    }
    // no change
    if (active_pt.ptr == pt.ptr) {
        return;
    }
    // if there's an old one we need to swap
    if (active_pt.ptr != 0) {
        swap_pt(active_pt, pt);
    }
    active_pt.ptr = pt.ptr;
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
        pt_entry_t pte_val = phys_pt_read(pt_paddr);
        // present bit set?
        if (flags_from_pte(&pte_val) & arch::vmm::FLAGS_PRESENT) {
            pt_paddr = paddr_from_pte(&pte_val);
            continue;
        }
        return status::StatusCode::EGENERIC;
    }
    return (arch::vmm::pte_t)pt_paddr;
}

std::pair<unsigned int, mm::paddr_t> arch::vmm::read_pte(arch::vmm::pte_t pte) {
    pt_entry_t pte_val = phys_pt_read(pte);

    return {
        .first = flags_from_pte(&pte_val),
        .second = paddr_from_pte(&pte_val),
    };
}

void arch::vmm::write_pte(
    arch::vmm::pte_t pte, mm::paddr_t phys, unsigned int flags
) {
    // NOTE: this won't work on this architecture if you expect it to actually map something.
    // It's kinda hard to make it work since we don't have a virtual address here
    pt_entry_t pte_val = 0;
    flags_into_pte(&pte_val, flags);
    paddr_into_pte(&pte_val, phys);

    phys_pt_write(pte, pte_val);
}

void arch::vmm::flush_tlb_single(mm::vaddr_t virt) {}

void arch::vmm::flush_tlb_all() {}
