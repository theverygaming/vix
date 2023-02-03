#include <arch/generic/memory.h>
#include <mm/allocators.h>
#include <mm/kvmm.h>
#include <panic.h>

#define KERNEL_PAGES ((ARCH_KERNEL_HEAP_END / ARCH_PAGE_SIZE) - (ARCH_KERNEL_HEAP_START / ARCH_PAGE_SIZE))

static mm::allocators::block_alloc_single<KERNEL_PAGES, ARCH_PAGE_SIZE> kvmm;

void mm::kv::init() {
    kvmm.init();
}

void *mm::kv::alloc(size_t pages) {
    bool success = false;
    void *allocated = kvmm.malloc(pages, &success);
    if (!success) {
        KERNEL_PANIC("kvmm -> memory full");
        return nullptr;
    }
    return ((uint8_t *)allocated) + ARCH_KERNEL_HEAP_START;
}

void mm::kv::free(void *address, size_t pages) {
    kvmm.dealloc(((uint8_t *)address) - ARCH_KERNEL_HEAP_START, pages);
}
