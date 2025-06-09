#pragma once
#include <vix/mm/mm.h>
#include <vix/config.h>
#include <vix/types.h>

#ifdef CONFIG_ARCH_HAS_PAGING
namespace arch::vmm {
    // if set the page is present in physical memory (not swapped out)
    inline const unsigned int FLAGS_PRESENT = (0x1 << 0);
    // if set the page has been accessed (written, read etc.)
    inline const unsigned int FLAGS_ACCESSED = (0x1 << 1);
    // if set the page has been written to
    inline const unsigned int FLAGS_DIRTY = (0x1 << 2);
    // if set page will not be cached
    inline const unsigned int FLAGS_CACHE_DISABLE = (0x1 << 3);
    // if set write-back caching will be enabled for this page
    inline const unsigned int FLAGS_WRITE_BACK = (0x1 << 4);
    // if set write-through caching will be enabled for this page
    inline const unsigned int FLAGS_WRITE_THROUGH = (0x1 << 5);
    // if set write-combined caching will be enabled for this page
    inline const unsigned int FLAGS_WRITE_COMBINING = (0x1 << 6);
    // if set the page can be accessed from usermode
    inline const unsigned int FLAGS_USER = (0x1 << 7);
    // if set the page will be read only to usermode and maybe even kernel mode
    inline const unsigned int FLAGS_READ_ONLY = (0x1 << 8);
    // if set the page cannot be executed as code
    inline const unsigned int FLAGS_NO_EXECUTE = (0x1 << 9);
    // if set the page will not be flushed when a syscall from usermode happens (on x86 this is the global bit) -- this should never be enabled for user pages, bad idea
    inline const unsigned int FLAGS_NO_FLUSH_ON_PRIV_CHANGE = (0x1 << 10);
    // flag that never gets touched by the MMU, usable for the OS for arbitrary purposes
    inline const unsigned int FLAGS_OS_FLAG_1 = (0x1 << 11);
    // flag that never gets touched by the MMU, usable for the OS for arbitrary purposes
    inline const unsigned int FLAGS_OS_FLAG_2 = (0x1 << 12);

    /**
     * gets flags and physical address for page
     * @param [in] virt virtual address of page
     * @param [out] fla
     * @return the physical address of the page
     */
    mm::paddr_t get_page(mm::vaddr_t virt, unsigned int *flags);

    /**
     * sets flags and physical address for page (a TLB flush may be required after this)
     * @param [in] virt virtual address of page
     * @param [in] phys physical address of page
     * @param [in] flags flags to set
     * @return previous flags
     */
    unsigned int set_page(mm::vaddr_t virt, mm::paddr_t phys, unsigned int flags);

    /**
     * flushes single page out of TLB
     * @param [in] virt virtual address of page
     */
    void flush_tlb_single(mm::vaddr_t virt);

    /**
     * flushes the entire TLB
     */
    void flush_tlb_all();
}
#endif
