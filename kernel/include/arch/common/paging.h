#pragma once
#include <config.h>
#include <types.h>

#ifdef CONFIG_ARCH_HAS_PAGING
namespace arch::vmm {
    static const unsigned int FLAGS_CACHE_DISABLE = (0x1 << 0); // if set page will not be cached
    static const unsigned int FLAGS_WRITE_THROUGH = (0x1 << 1); // if set write-through will be enabled for this page
    static const unsigned int FLAGS_USER = (0x1 << 2);          // if set the page can be accessed from usermode
    static const unsigned int FLAGS_READ_ONLY = (0x1 << 3);     // if set the page will be read only to usermode and maybe even kernel mode
    static const unsigned int FLAGS_NO_EXECUTE = (0x1 << 4);    // if set the page cannot be executed as code

    /**
     * maps a single page and flushes it from TLB
     * @param [in] virt virtual address of page to be mapped
     * @param [in] phys physical address of page to be mapped
     * @param [in] flags constants arch::vmm::FLAGS_* ORed together
     */
    void map_page(uintptr_t virt, uintptr_t phys, unsigned int flags);

    /**
     * maps virtually and physically contiguous pages, **does not flush TLB**
     * @param [in] virt virtual start address of pages to be mapped
     * @param [in] phys physical start address of pages to be mapped
     * @param [in] count amount of pages to be mapped (virtual and physical will be incremented by one page size)
     * @param [in] flags constants arch::vmm::FLAGS_* ORed together
     */
    void map_contiguous(uintptr_t virt, uintptr_t phys, size_t count, unsigned int flags, bool flush_full_tlb);

    /**
     * unmaps page and flushes it from TLB
     * @param [in] virt virtual address of page to be unmapped
     * @param [out] phys can be nullptr, if not it will contain the physical address of the unmapped page
     * @return true if page was mapped (if not the value in \p phys must be disregarded)
     */
    bool unmap_page(uintptr_t virt, uintptr_t *phys);

    /**
     * gets flags and physical address for page
     * @param [in] virt virtual address of page
     * @param [out] phys physical address of page
     * @param [out] flags constants arch::vmm::FLAGS_* ORed together
     * @return if page was mapped (if not \p phys and \p flags must be disregarded)
     */
    bool get_page(uintptr_t virt, unsigned int *phys, unsigned int *flags); // returns true if page is mapped

    /**
     * flushes single page out of TLB
     * @param [in] virt virtual address of page
     */
    void flush_tlb_single(uintptr_t virt);

    /**
     * flushes the entire TLB
     */
    void flush_tlb_all();

    /**
     * gets dirty bit
     * @param [in] virt virtual address of page
     * @return true if page \p virt is dirty, always returns false if \p virt is not mapped
     */
    bool dirty_get(uintptr_t virt);

    /**
     * clears dirty bit of page \p virt
     * @param [in] virt virtual address of page
     * @return true if dirty bit of \p virt was set, always returns false if \p virt is not mapped
     */
    bool dirty_clear(uintptr_t virt);
}
#endif
