#include <vix/arch/common/paging.h>
#include <vix/config.h>
#include <vix/macros.h>
#include <vix/mm/mm.h>
#include <vix/mm/pmm.h>
#include <vix/mm/vmm.h>
#include <vix/panic.h>
#include <vix/status.h>

#ifdef CONFIG_ARCH_HAS_PAGING
static unsigned int get_vm_extra_flags(mm::alloc_attrs attrs) {
    unsigned int vm_flags = 0;
    switch (attrs.cache) {
    case mm::caching_type::DISABLED:
        vm_flags |= arch::vmm::FLAGS_CACHE_DISABLE;
        break;
    case mm::caching_type::WRITE_COMBINING:
        vm_flags |= arch::vmm::FLAGS_WRITE_COMBINING;
        break;
    case mm::caching_type::WRITE_THROUGH:
        vm_flags |= arch::vmm::FLAGS_WRITE_THROUGH;
        break;
    case mm::caching_type::WRITE_BACK:
        vm_flags |= arch::vmm::FLAGS_WRITE_BACK;
        break;
    }

    if (attrs.user) {
        vm_flags |= arch::vmm::FLAGS_USER;
    }
    if (attrs.read_only) {
        vm_flags |= arch::vmm::FLAGS_READ_ONLY;
    }
    if (attrs.no_execute) {
        vm_flags |= arch::vmm::FLAGS_NO_EXECUTE;
    }

    return vm_flags;
}
#endif

status::StatusOr<void *> mm::map_arbitrary_phys(
    paddr_t phys,
    size_t bytes,
    alloc_attrs attrs,
    // FIXME: this should not be ignored!!! (NOTE: when done update comments in mm.h)
    vaddr_range vrange
) {
    size_t pages =
        (ALIGN_UP(bytes, CONFIG_ARCH_PAGE_SIZE)) / CONFIG_ARCH_PAGE_SIZE;
#ifdef CONFIG_ARCH_HAS_PAGING
    unsigned int vm_flags =
        arch::vmm::FLAGS_PRESENT | get_vm_extra_flags(attrs);

    // TODO: this does NOT do user memory at all
    vaddr_t area = mm::vmm::kalloc(pages);
    for (size_t i = 0; i < pages; i++) {
        uintptr_t virt = area + (i * CONFIG_ARCH_PAGE_SIZE);
        arch::vmm::set_page(virt, phys + (i * CONFIG_ARCH_PAGE_SIZE), vm_flags);
        arch::vmm::flush_tlb_single(virt);
    }
    return (void *)area;
#else
    return (void *)phys;
#endif
}

void mm::unmap_arbitrary_phys(void *addr, size_t bytes) {
#ifdef CONFIG_ARCH_HAS_PAGING
    size_t n_pages =
        (ALIGN_UP(bytes, CONFIG_ARCH_PAGE_SIZE)) / CONFIG_ARCH_PAGE_SIZE;
    for (size_t i = 0; i < n_pages; i++) {
        if (unlikely(
                !(arch::vmm::set_page(
                      ((vaddr_t)addr) + (i * CONFIG_ARCH_PAGE_SIZE), 0, 0
                  ) &
                  arch::vmm::FLAGS_PRESENT)
            )) {
            KERNEL_PANIC("unmap_arbitrary_phys page is somehow unmapped");
        }
    }
#endif
}

status::StatusOr<void *> mm::allocate_non_contiguous(
    size_t bytes,
    alloc_attrs attrs,
    // FIXME: this should not be ignored!!! (NOTE: when done update comments in mm.h)
    paddr_range prange,
    // FIXME: this should not be ignored!!! (NOTE: when done update comments in mm.h)
    vaddr_range vrange
) {
    size_t pages =
        (ALIGN_UP(bytes, CONFIG_ARCH_PAGE_SIZE)) / CONFIG_ARCH_PAGE_SIZE;
#ifdef CONFIG_ARCH_HAS_PAGING
    unsigned int vm_flags =
        arch::vmm::FLAGS_PRESENT | get_vm_extra_flags(attrs);

    // TODO: this does NOT do user memory at all
    vaddr_t area = mm::vmm::kalloc(pages);
    for (size_t i = 0; i < pages; i++) {
        auto phys_alloc = mm::pmm::alloc_contiguous(1);
        if (!phys_alloc.status().ok()) {
            // fuck! now we need to clean up the mess we made!

            // j < i because we did not do anything in the current run
            for (size_t j = 0; j < i; j++) {
                uintptr_t virt = area + (j * CONFIG_ARCH_PAGE_SIZE);
                unsigned int tmp;
                paddr_t phys = arch::vmm::get_page(virt, &tmp);
                mm::pmm::free_contiguous(phys, 1);
                arch::vmm::set_page(virt, 0, 0);
                arch::vmm::flush_tlb_single(virt);
            }
            return phys_alloc.status().code();
        }
        uintptr_t virt = area + (i * CONFIG_ARCH_PAGE_SIZE);
        arch::vmm::set_page(virt, phys_alloc.value(), vm_flags);
        arch::vmm::flush_tlb_single(virt);
    }
    return (void *)area;
#else
    paddr_t area;
    ASSIGN_OR_RETURN(area, mm::pmm::alloc_contiguous(pages));
    return (void *)area;
#endif
}

void mm::free_non_contiguous(void *addr, size_t bytes) {
    size_t n_pages =
        (ALIGN_UP(bytes, CONFIG_ARCH_PAGE_SIZE)) / CONFIG_ARCH_PAGE_SIZE;

#ifdef CONFIG_ARCH_HAS_PAGING
    unsigned int flags;
    for (size_t i = 0; i < n_pages; i++) {
        paddr_t phys = arch::vmm::get_page(
            ((vaddr_t)addr) + (i * CONFIG_ARCH_PAGE_SIZE), &flags
        );
        if (unlikely(
                !(arch::vmm::set_page(
                      ((vaddr_t)addr) + (i * CONFIG_ARCH_PAGE_SIZE), 0, 0
                  ) &
                  arch::vmm::FLAGS_PRESENT)
            )) {
            KERNEL_PANIC("free_non_contiguous page is somehow unmapped");
        }
        mm::pmm::free_contiguous(phys, 1);
    }
#else
    for (size_t i = 0; i < n_pages; i++) {
        mm::pmm::free_contiguous(
            ((paddr_t)address) + (i * CONFIG_ARCH_PAGE_SIZE), 1
        );
    }
#endif
}

status::StatusOr<void *> mm::allocate_contiguous(
    size_t bytes,
    alloc_attrs attrs,
    // FIXME: this should not be ignored!!! (NOTE: when done update comments in mm.h)
    paddr_range prange,
    // FIXME: this should not be ignored!!! (NOTE: when done update comments in mm.h)
    vaddr_range vrange
) {
    size_t pages =
        ALIGN_UP(bytes, CONFIG_ARCH_PAGE_SIZE) / CONFIG_ARCH_PAGE_SIZE;
#ifdef CONFIG_ARCH_HAS_PAGING
    paddr_t phys;
    ASSIGN_OR_RETURN(phys, mm::pmm::alloc_contiguous(pages));
    vaddr_t area = mm::vmm::kalloc(pages);
    for (size_t i = 0; i < pages; i++) {
        uintptr_t virt = ((uintptr_t)area) + (i * CONFIG_ARCH_PAGE_SIZE);
        arch::vmm::set_page(
            virt,
            (uintptr_t)phys + (i * CONFIG_ARCH_PAGE_SIZE),
            arch::vmm::FLAGS_PRESENT
        );
        arch::vmm::flush_tlb_single(virt);
    }
    return (void *)area;
#else
    paddr_t area;
    ASSIGN_OR_RETURN(area, mm::pmm::alloc_contiguous(pages));
    return (void *)area;
#endif
}

void mm::free_contiguous(void *addr, size_t bytes) {
    mm::free_non_contiguous(addr, bytes); // that'll work fine
}
