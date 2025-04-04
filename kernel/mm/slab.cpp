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
static void *alloc_pages(size_t pages) {
    void *area;
    ASSIGN_OR_PANIC(area, mm::allocate_non_contiguous(pages * CONFIG_ARCH_PAGE_SIZE));
    return area;
}

static void free_pages(void *address, size_t count) {
    mm::free_non_contiguous(address, count * CONFIG_ARCH_PAGE_SIZE);
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
        if (ALIGN_ANY_UP(sizeof(slab) + sizeof(cache), obj_size) <= CONFIG_ARCH_PAGE_SIZE) { // TODO: check for off-by-one error
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
DEFINE_INITCALL(INITCALL_AFTER_MM_INIT, INITCALL_PRIO_NORMAL, test_slab);
