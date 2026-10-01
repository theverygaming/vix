#include "vix/debug.h"
#include "vix/kprintf.h"
#include "vix/macros.h"
#include "vix/mm/mm.h"
#include "vix/panic.h"
#include <vix/config.h>
#include <vix/arch/common/paging.h>
#include <vix/arch/linux-syscalls.h>
#include <vix/arch/paging.h>

// TODO: we currently only implement set_page and get_page for the kernel MM to work. Userspace stuff is still TODO!

extern int memfd;
extern size_t memfd_bytes;

static int proc_maps_fd;

void paging_init() {
    proc_maps_fd = linux_open("/proc/self/maps", LINUX_O_RDONLY, 0);
    if (proc_maps_fd < 0) {
        KERNEL_PANIC("got error %d", proc_maps_fd);
    }
}

static unsigned int pmq_get_flags(struct linux_procmap_query *q) {
    unsigned int flags = arch::vmm::FLAGS_PRESENT;
    if ((q->vma_flags & LINUX_PROCMAP_QUERY_VMA_READABLE) != 0 && (q->vma_flags & LINUX_PROCMAP_QUERY_VMA_WRITABLE) == 0) {
        flags |= arch::vmm::FLAGS_READ_ONLY;
    }
    if ((q->vma_flags & LINUX_PROCMAP_QUERY_VMA_EXECUTABLE) == 0) {
        flags |= arch::vmm::FLAGS_NO_EXECUTE;
    }
    return flags;
}

mm::paddr_t arch::vmm::get_page(mm::vaddr_t virt, unsigned int *flags) {
    if (!IS_ALIGNED(virt, CONFIG_ARCH_PAGE_SIZE)) {
        KERNEL_PANIC("virt is not aligned. virt: 0x%p", virt);
    }
    struct linux_procmap_query query = {
        .size = sizeof(struct linux_procmap_query),
        .query_flags = 0, // in
        .query_addr = virt, // in
        .vma_start = 0,
        .vma_end = 0,
        .vma_flags = 0,
        .vma_page_size = 0,
        .vma_offset = 0,
        .inode = 0,
        .dev_major = 0,
        .dev_minor = 0,
        .vma_name_size = 0, // in/out
        .build_id_size = 0, // in/out
        .vma_name_addr = 0, // in
        .build_id_addr = 0, // in
    };
    if (linux_ioctl(proc_maps_fd, LINUX_PROCMAP_QUERY, (unsigned long)&query) < 0) {
        // unmapped
        if (flags != nullptr) {
            *flags = 0;
        }
        return 0;
    }
    if (query.inode == 0) {
        KERNEL_PANIC("no backing file");
    }
    if (!(virt >= query.vma_start && virt <= query.vma_end)) {
        KERNEL_PANIC("wrong mapping returned. Asked 0x%p got 0x%p-0x%p", virt, query.vma_start, query.vma_end);
    }
    if (flags != nullptr) {
        *flags = pmq_get_flags(&query);
    }
    return query.vma_offset + (virt - query.vma_start); // account for merging
}

unsigned int arch::vmm::set_page(mm::vaddr_t virt, mm::paddr_t phys, unsigned int flags) {
    if (!IS_ALIGNED(virt, CONFIG_ARCH_PAGE_SIZE) || !IS_ALIGNED(phys, CONFIG_ARCH_PAGE_SIZE)) {
        KERNEL_PANIC("virt or phys are not aligned. virt: 0x%p phys: 0x%p", virt, phys);
    }
    unsigned int flags_old;
    mm::paddr_t phys_old = get_page(virt, &flags_old);
    void *virt_tgt = (void *)virt;
    if ((flags & arch::vmm::FLAGS_PRESENT) != 0) {
        if (phys > memfd_bytes) {
            KERNEL_PANIC("attempted mapping outside physical address space 0x%p only have 0x%p", phys, memfd_bytes);
        }
        int prot = LINUX_PROT_READ;
        if ((flags & arch::vmm::FLAGS_READ_ONLY) == 0) {
            prot |= LINUX_PROT_WRITE;
        }
        if ((flags & arch::vmm::FLAGS_NO_EXECUTE) == 0) {
            prot |= LINUX_PROT_EXEC;
        }
        if (linux_mmap(virt_tgt, CONFIG_ARCH_PAGE_SIZE, prot, LINUX_MAP_SHARED | LINUX_MAP_FIXED, memfd, phys) != virt_tgt) {
            KERNEL_PANIC("mmap failed");
        }
        // FIXME: debugging stuff
        unsigned int flags_test;
        mm::paddr_t paddr_test = get_page(virt, &flags_test);
        if ((flags & arch::vmm::FLAGS_WRITE_BACK) != 0) {
            flags_test |= arch::vmm::FLAGS_WRITE_BACK;
        }
        if (paddr_test != phys || flags_test != flags) {
            KERNEL_PANIC("expected (vaddr 0x%p): phys: 0x%p flags: 0x%p got: phys: 0x%p flags: 0x%p prev: phys: 0x%p flags: 0x%p", virt_tgt, phys, flags, paddr_test, flags_test, phys_old, flags_old);
        } else {
            //DEBUG_PRINTF("OK! expected (vaddr 0x%p): phys: 0x%p flags: 0x%p got: phys: 0x%p flags: 0x%p prev: phys: 0x%p flags: 0x%p\n", virt_tgt, phys, flags, paddr_test, flags_test, phys_old, flags_old);
        }
    } else {
        //DEBUG_PRINTF("munmap: 0x%p len 0x%p\n", virt_tgt, CONFIG_ARCH_PAGE_SIZE);
        linux_munmap(virt_tgt, CONFIG_ARCH_PAGE_SIZE);
    }
    return flags_old;
}

void arch::vmm::flush_tlb_single(mm::vaddr_t virt) {}

void arch::vmm::flush_tlb_all() {}
