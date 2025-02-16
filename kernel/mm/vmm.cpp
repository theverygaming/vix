#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/kprintf.h>
#include <vix/mm/allocators.h>
#include <vix/mm/vmm.h>
#include <vix/panic.h>

#ifdef CONFIG_ARCH_HAS_PAGING
void mm::vmm::init() {
    // unmap all that nonsense the prekernel does...
    // FIXME: fix that x86 prekernel paging nonsense
    void *addr = (void *)ARCH_KERNEL_HEAP_START;
    // BUG: - CONFIG_ARCH_PAGE_SIZE due to seemingly conflicts with the multiboot2 framebuffer thing??????
    while ((uintptr_t)addr < (ARCH_KERNEL_HEAP_END - CONFIG_ARCH_PAGE_SIZE)) {
        arch::vmm::set_page((uintptr_t)addr, 0, 0);
        arch::vmm::flush_tlb_single((uintptr_t)addr);
        addr = (void *)((uintptr_t)addr + CONFIG_ARCH_PAGE_SIZE);
    }
    kprintf(KP_INFO, "vmm: initialized virtual memory manager\n");
}

// NOTE: this could be improved a little by keeping the last highest allocation and the last highest free
// currently this will take longer and longer the more it searches
static void *find_free_pages(void *start, void *end, size_t pages) {
    size_t pages_found = 0;
    void *start_found = nullptr;
    while ((uintptr_t)start < (uintptr_t)end && pages_found != pages) {
        unsigned int flags;
        arch::vmm::get_page((uintptr_t)start, &flags);
        if (!(flags & arch::vmm::FLAGS_PRESENT)) {
            if (pages_found == 0) {
                start_found = start;
            }
            pages_found += 1;
        } else {
            pages_found = 0;
        }
        start = (void *)((uintptr_t)start + CONFIG_ARCH_PAGE_SIZE);
    }
    if (pages_found != pages) {
        KERNEL_PANIC("ran out of VMM space");
    }
    return start_found;
}

void *mm::vmm::alloc(void *start, void *end, size_t pages) {
    void *addr = find_free_pages(start, end, pages);
    return addr;
}

void *mm::vmm::kalloc(size_t pages) {
    return mm::vmm::alloc((void *)ARCH_KERNEL_HEAP_START, (void *)ARCH_KERNEL_HEAP_END, pages);
}

void mm::vmm::dealloc(void *start, size_t n) {
    KERNEL_PANIC("not implemented");
}

#endif
