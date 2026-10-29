#pragma once
#include <vix/mm/mm_types.h>
#include <vix/status.h>
#include <vix/types.h>
#include <vix/arch/generic/memory.h>
#include <vix/config.h>
#include <forward_list>
#include <vix/arch/common/paging.h>

namespace mm {
    inline const paddr_range PRANGE_DEFAULT = {0, PADDR_MAX};

    // memory caching type
    enum class caching_type {
        DISABLED,
        WRITE_COMBINING,
        WRITE_THROUGH,
        WRITE_BACK,
    };

    // memory allocation attributes
    struct alloc_attrs {
        bool user;
        bool read_only;
        bool no_execute;
        caching_type cache;
    };

    status::StatusOr<void *> map_arbitrary_phys(
        paddr_t phys,
        size_t bytes,
        alloc_attrs attrs = {false, false, false, caching_type::WRITE_BACK},
#ifdef CONFIG_ARCH_HAS_PAGING
        vaddr_range vrange = {.start = ARCH_KERNEL_HEAP_START, .end = ARCH_KERNEL_HEAP_END}
#else
        vaddr_range vrange = {0, VADDR_MAX}
#endif
    );
    void unmap_arbitrary_phys(void *addr, size_t bytes);

    status::StatusOr<void *> allocate_contiguous(
        size_t bytes,
        alloc_attrs attrs = {false, false, false, caching_type::WRITE_BACK},
        paddr_range prange = PRANGE_DEFAULT,
#ifdef CONFIG_ARCH_HAS_PAGING
        vaddr_range vrange = {.start = ARCH_KERNEL_HEAP_START, .end = ARCH_KERNEL_HEAP_END}
#else
        vaddr_range vrange = {0, VADDR_MAX}
#endif
    );
    void free_contiguous(void *addr, size_t bytes);

    status::StatusOr<void *> allocate_non_contiguous(
        size_t bytes,
        alloc_attrs attrs = {false, false, false, caching_type::WRITE_BACK},
        paddr_range prange = PRANGE_DEFAULT,
#ifdef CONFIG_ARCH_HAS_PAGING
        vaddr_range vrange = {.start = ARCH_KERNEL_HEAP_START, .end = ARCH_KERNEL_HEAP_END}
#else
        vaddr_range vrange = {0, VADDR_MAX}
#endif
    );
    void free_non_contiguous(void *addr, size_t bytes);

    struct mem_area {
        struct aaddr_range arange;
        struct alloc_attrs attrs;
    };

    typedef struct mem_areas_struct {
        std::forward_list<struct mem_area> areas;
#ifdef CONFIG_ARCH_HAS_PAGING
        arch::vmm::pt_t pt;
#endif
    } mem_areas_t;

    status::Status<> mem_areas_init(mem_areas_t *areas);
    void mem_areas_deinit(mem_areas_t *areas);

    status::StatusOr<void *> mem_areas_alloc_range(
        mem_areas_t *areas,
        size_t min_bytes,
        struct alloc_attrs attrs,
#ifdef CONFIG_ARCH_HAS_PAGING
        aaddr_range arange = {.start = 0, .end = CONFIG_KERNEL_HIGHER_HALF}
#else
        aaddr_range arange = {0, AADDR_MAX}
#endif
    );
    // FIXME: implement freeing lmao

    // Temporarily map the given areas into memory (e.g. while loading an application).
    // returns a token that must be passed back to mem_areas_tmp_restore
    void *mem_areas_tmp_load(mem_areas_t *areas);
    void mem_areas_tmp_restore(void *token);
}
