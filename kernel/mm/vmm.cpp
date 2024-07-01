#include <arch/common/paging.h>
#include <arch/generic/memory.h>
#include <config.h>
#include <kprintf.h>
#include <mm/allocators.h>
#include <mm/vmm.h>
#include <panic.h>

#ifdef CONFIG_ARCH_HAS_PAGING
void mm::vmm::init() {
    // unmap all that nonsense the prekernel does...
    // TODO: fix that x86 prekernel paging nonsense
    void *addr = (void *)ARCH_KERNEL_HEAP_START;
    // BUG: ARCH_PAGE_SIZE * 1 due to seemingly conflicts with the multiboot2 framebuffer thing??????
    while ((uintptr_t)addr < (ARCH_KERNEL_HEAP_END - (ARCH_PAGE_SIZE * 1))) {
        uintptr_t phys;
        arch::vmm::unmap_page((uintptr_t)addr, &phys);
        addr = (void *)((uintptr_t)addr + ARCH_PAGE_SIZE);
    }
    kprintf(KP_INFO, "vmm: initialized virtual memory manager\n");
}

// NOTE: this could be improved a little by keeping the last highest allocation and the last highest free
// currently this will take longer and longer the more it searches
static void *find_free_pages(void *start, void *end, size_t pages) {
    size_t pages_found = 0;
    void *start_found = nullptr;
    while ((uintptr_t)start < (uintptr_t)end && pages_found != pages) {
        uintptr_t phys;
        unsigned int flags;
        bool mapped = arch::vmm::get_page((uintptr_t)start, &phys, &flags);
        if (!mapped) {
            if (pages_found == 0) {
                start_found = start;
            }
            pages_found += 1;
        } else {
            pages_found = 0;
        }
        start = (void *)((uintptr_t)start + ARCH_PAGE_SIZE);
    }
    if (pages_found != pages) {
        KERNEL_PANIC("ran out of VMM space");
    }
    return start_found;
}

void *mm::vmm::alloc(void *start, void *end, size_t pages) {
    void *addr = find_free_pages(start, end, pages);
    kprintf(KP_INFO, "vmm: alloc(0x%p, 0x%p, %u) -> 0x%p\n", start, end, pages);
    return addr;
}

void *mm::vmm::kalloc(size_t pages) {
    return mm::vmm::alloc((void *)ARCH_KERNEL_HEAP_START, (void *)ARCH_KERNEL_HEAP_END, pages);
}

void mm::vmm::dealloc(void *start, size_t n) {
    KERNEL_PANIC("not implemented");
}

#endif
