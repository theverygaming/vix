#include <vix/arch/common/paging.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <vix/initcall.h> // FIXME: for testing only!
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/mm/pmm.h>
#include <vix/mm/slab.h>
#include <vix/mm/vmm.h>
#include <vix/status.h>

/*
 * Page allocator
 */

// TODO: check if commonly used, move in that case

static void *alloc_pages(size_t pages) {
#ifdef CONFIG_ARCH_HAS_PAGING
    void *area = mm::vmm::kalloc(pages);
    for (size_t i = 0; i < pages; i++) {
        void *phys;
        ASSIGN_OR_PANIC(phys, mm::pmm::alloc_contiguous(1));
        uintptr_t virt = ((uintptr_t)area) + (i * ARCH_PAGE_SIZE);
        arch::vmm::set_page(virt, (uintptr_t)phys, arch::vmm::FLAGS_PRESENT);
        arch::vmm::flush_tlb_single(virt);
    }
#else
    void *area;
    ASSIGN_OR_PANIC(area, mm::pmm::alloc_contiguous(pages));
#endif
    return area;
}

static void free_pages(void *address, size_t count) {
#ifdef CONFIG_ARCH_HAS_PAGING
    unsigned int flags;
    for (size_t i = 0; i < count; i++) {
        uintptr_t phys = arch::vmm::get_page(((uintptr_t)address) + (i * ARCH_PAGE_SIZE), &flags);
        if (unlikely(!(arch::vmm::set_page(((uintptr_t)address) + (i * ARCH_PAGE_SIZE), 0, 0) & arch::vmm::FLAGS_PRESENT))) {
            KERNEL_PANIC("kmalloc page is somehow unmapped");
        }
        mm::pmm::free_contiguous((void *)phys, 1);
    }
#else
    for (size_t i = 0; i < count; i++) {
        mm::pmm::free_contiguous(((uint8_t *)address) + (i * ARCH_PAGE_SIZE), 1);
    }
#endif
}

/*
 * Slab layout in memory:
 * First slab (one slab can span multiple blocks/pages in size, that's what n_blocks is for):
 * Block/Page 0: | struct slab | struct cache | padding | object | free (slab_free) |
 * Block/Page 1: | object | free (slab_free) | padding |
 * 
 * Second and further slabs:
 * Block/Page 0: | struct slab | padding | object | free (slab_free) |
 * Block/Page 1: | object | free (slab_free) | padding |
 */

namespace mm::slab {
    status::StatusOr<struct cache *> create_cache(const char *obj_name, size_t obj_size) {
        // check if things will fit into the first block/page, if they don't we might as well abort here
        if (ALIGN_ANY_UP(sizeof(slab) + sizeof(cache), obj_size) <= ARCH_PAGE_SIZE) { // TODO: check for off-by-one error
            return status::StatusCode::UNKNOWN_ERR;
        }
        void *first_page = alloc_pages(1);
        struct slab *first = (struct slab *)first_page;
        struct cache *cache = (struct cache *)first_page;

        return nullptr;
    }
}

// FIXME: for testing only!
static int test_slab() {
    kprintf(KP_INFO, "slab: test start\n");
    auto cache_st = mm::slab::create_cache("test object", 4096);
    if (!cache_st.status().ok()) {
        kprintf(KP_INFO, "slab: test NOT OK!\n");
    }
    struct mm::slab::cache *cache = cache_st.value();
    kprintf(KP_INFO, "slab: test end\n");
    return 0;
}
INITCALL(test_slab);
