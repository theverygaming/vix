#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/kprintf.h>
#include <vix/mm/allocators.h>
#include <vix/mm/mm.h>
#include <vix/mm/vmm.h>
#include <vix/panic.h>

void mm::vmm::init() {
    kprintf(KP_INFO, "vmm: initialized virtual memory manager\n");
}

// NOTE: this could be improved a little by keeping the last highest allocation and the last highest free
// currently this will take longer and longer the more it searches
static mm::vaddr_t find_free_pages(mm::vaddr_range range, size_t pages) {
    mm::vaddr_t start = range.start;
    mm::vaddr_t end = range.end;
    size_t pages_found = 0;
    mm::vaddr_t start_found = 0;
    while ((uintptr_t)start < (uintptr_t)end && pages_found != pages) {
        unsigned int flags;
        arch::vmm::get_page_pt(arch::vmm::get_active_pt(), (uintptr_t)start, &flags);
        if (!(flags & arch::vmm::FLAGS_PRESENT)) {
            if (pages_found == 0) {
                start_found = start;
            }
            pages_found += 1;
        } else {
            pages_found = 0;
        }
        start = (mm::vaddr_t)start + CONFIG_ARCH_PAGE_SIZE;
    }
    if (pages_found != pages) {
        KERNEL_PANIC("ran out of VMM space");
    }
    return start_found;
}

mm::vaddr_t mm::vmm::find_free(mm::vaddr_range range, size_t pages) {
    vaddr_t addr = find_free_pages(range, pages);
    return addr;
}
