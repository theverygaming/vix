#pragma once
#include <vix/status.h>
#include <vix/types.h>

namespace mm {
    // physical address
    typedef uintptr_t paddr_t;
    // virtual address
    typedef uintptr_t vaddr_t;

    // physical memory range
    struct paddr_range {
        paddr_t start;
        paddr_t end;
    };

    // virtual memory range
    struct vaddr_range {
        vaddr_t start;
        vaddr_t end;
    };

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
        // FIXME: this should actually only be kernel memory, also currently it's ignored :sob:
        vaddr_range vrange = {0, UINTPTR_MAX}
    );
    void unmap_arbitrary_phys(void *addr, size_t bytes);

    status::StatusOr<void *> allocate_contiguous(
        size_t bytes,
        alloc_attrs attrs = {false, false, false, caching_type::WRITE_BACK},
        // FIXME: this should not be ignored!!!
        paddr_range prange = {0, UINTPTR_MAX},
        // FIXME: this should actually only be kernel memory, also currently it's ignored :sob:
        vaddr_range vrange = {0, UINTPTR_MAX}
    );
    void free_contiguous(void *addr, size_t bytes);

    status::StatusOr<void *> allocate_non_contiguous(
        size_t bytes,
        alloc_attrs attrs = {false, false, false, caching_type::WRITE_BACK},
        // FIXME: this should not be ignored!!!
        paddr_range prange = {0, UINTPTR_MAX},
        // FIXME: this should actually only be kernel memory, also currently it's ignored :sob:
        vaddr_range vrange = {0, UINTPTR_MAX}
    );
    void free_non_contiguous(void *addr, size_t bytes);
}
