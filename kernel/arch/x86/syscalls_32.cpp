#include <arch/drivers/ps2.h>
#include <arch/elf.h>
#include <arch/errno.h>
#include <arch/gdt.h>
#include <arch/generic/memory.h>
#include <arch/memorymap.h>
#include <arch/multitasking.h>
#include <arch/paging.h>
#include <arch/syscalls_32.h>
#include <cppstd/string.h>
#include <debug.h>
#include <fs/vfs.h>
#include <generated/autoconf.h>
#include <log.h>
#include <mm/kmalloc.h>
#include <mm/phys.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint32_t sys_dbg(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t n, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_dbg");
    if (n == 1) {
        multitasking::list_processes();
    }
    return 0;
}

uint32_t sys_exit(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t exit_code, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 0;
    DEBUG_PRINTF("syscall: sys_exit code: %d\n", exit_code);
    multitasking::killCurrentProcess(regs);
    return 0;
}

uint32_t sys_fork(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_fork");
    multitasking::x86_process *newprocess = multitasking::fork_current_process(regs);
    if (newprocess) {
        newprocess->registerContext.eax = 0;
        return newprocess->tgid;
    } else {
        return -EAGAIN;
    }
}

uint32_t sys_write(isr::registers *, int *syscall_ret, uint32_t, uint32_t fd, uint32_t _buf, uint32_t count, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    char *buf = (char *)_buf;

    DEBUG_PRINTF("syscall: sys_write count: %u\n", count);
    if (!paging::is_readable(buf + count)) {
        return -1; // TODO: proper return value
    }
    // char string[count + 1];
    char *string = (char *)__builtin_alloca((count + 1));
    string[count] = '\0';
    memcpy(string, buf, count);
    printf("%s", string);
    return count; // return number of written bytes
}

uint32_t sys_open(isr::registers *, int *syscall_ret, uint32_t, uint32_t _filename, uint32_t _flags, uint32_t _mode, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_open");
    const char *filename = (const char *)_filename;
    DEBUG_PRINTF("open: %s\n", filename);
    return -ENOENT;
}

uint32_t sys_close(isr::registers *, int *syscall_ret, uint32_t, uint32_t _fd, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_close");
    int fd = (int)_fd;
    DEBUG_PRINTF("close: %d\n", fd);
    return -EBADF;
}

uint32_t sys_execve(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t _filename, uint32_t _argv, uint32_t _envp, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 0;
    char *filename = (char *)_filename;
    const char *const *argv = (const char *const *)_argv;
    // const char *const *envp = (const char *const *)_envp;
    DEBUG_PRINTF("syscall: sys_execve -> %s\n", filename);
    void *elfptr;
    if (fs::vfs::fptr(filename, &elfptr)) {
        std::vector<std::string> args;
        // copy argv to vector
        int argc = 0;
        while (argv[argc] != nullptr) {
            args.push_back(argv[argc]);
            argc++;
        }
        elf::load_program(elfptr, &args, true, multitasking::getCurrentProcess()->tgid, regs);
        return 0;
    }
    return -1;
}

uint32_t sys_time(isr::registers *, int *syscall_ret, uint32_t, uint32_t _tloc, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    uint64_t *tloc = (uint64_t *)_tloc;
    *tloc = time::getCurrentUnixTime();
    return 0;
}

uint32_t sys_getpid(isr::registers *, int *syscall_ret, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_getpid");
    return multitasking::getCurrentProcess()->tgid;
}

uint32_t sys_brk(isr::registers *, int *syscall_ret, uint32_t, uint32_t _brk, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    DEBUG_PRINTF("syscall: sys_brk -> 0x%p\n", _brk);
    // hacky level: insane
    /*
     * list of things to improve here:
     * - actually keep track of the break
     * - don't allocate a whole page every time
     * - sleep more
     */
    multitasking::x86_process *process = multitasking::getCurrentProcess();
    // hack: compute break size
    size_t brk_pages = 0;
    for (size_t i = 0; i < process->pages.size(); i++) {
        if (process->pages[i].type == multitasking::process_pagerange::range_type::BREAK) {
            brk_pages += process->pages[i].pages;
        }
    }
    uintptr_t current_break_adr = process->brk_start + (brk_pages * ARCH_PAGE_SIZE);

    if (_brk == 0) {
        return current_break_adr;
    }

    if (current_break_adr >= _brk) {
        return _brk; // we never actually deallocate :troll:
    } else {
        uint32_t newbrk = _brk;
        if (newbrk % ARCH_PAGE_SIZE != 0) { // align brk with page
            newbrk += ARCH_PAGE_SIZE - (newbrk % ARCH_PAGE_SIZE);
        }
        size_t needed_pages = (newbrk - current_break_adr) / ARCH_PAGE_SIZE;
        process->pages.push_back({.phys_base = (uint32_t)mm::phys::phys_malloc(needed_pages),
                                  .virt_base = process->brk_start + (brk_pages * ARCH_PAGE_SIZE),
                                  .pages = needed_pages,
                                  .type = multitasking::process_pagerange::range_type::BREAK});
        multitasking::setPageRange(&process->pages);
    }
    return _brk;
}

uint32_t sys_ioctl(isr::registers *, int *syscall_ret, uint32_t, uint32_t _brk, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    DEBUG_PRINTF("syscall: sys_ioctl\n");
    return 0;
}

uint32_t sys_mmap(isr::registers *, int *syscall_ret, uint32_t, uint32_t mmap_struct_ptr, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_mmap -- unstable");
    typedef struct {
        void *start;
        uint32_t length;
        int prot;
        int flags;
        int fd;
        int offset;
    } mmap_args_t;
    mmap_args_t *args = (mmap_args_t *)mmap_struct_ptr;

    uint8_t *alloc_adr = (uint8_t *)0xB69420;
    DEBUG_PRINTF("mmap: program wants %u bytes\n", args->length);

    int pages = (args->length / ARCH_PAGE_SIZE) + 1;
    uint8_t *map = (uint8_t *)mm::phys::phys_malloc(pages);
    for (int i = 0; i < pages; i++) {
        paging::map_page(map + (i * ARCH_PAGE_SIZE), alloc_adr + (i * ARCH_PAGE_SIZE));
    }
    multitasking::refresh_current_process_pagerange();

    return (uint32_t)alloc_adr;
}

uint32_t sys_sysinfo(isr::registers *, int *syscall_ret, uint32_t, uint32_t _info, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    // https://man7.org/linux/man-pages/man2/sysinfo.2.html
    struct sysinfo {
        long uptime;                                  /* Seconds since boot */
        unsigned long loads[3];                       /* 1, 5, and 15 minute load averages */
        unsigned long totalram;                       /* Total usable main memory size */
        unsigned long freeram;                        /* Available memory size */
        unsigned long sharedram;                      /* Amount of shared memory */
        unsigned long bufferram;                      /* Memory used by buffers */
        unsigned long totalswap;                      /* Total swap space size */
        unsigned long freeswap;                       /* Swap space still available */
        unsigned short procs;                         /* Number of current processes */
        unsigned long totalhigh;                      /* Total high memory size */
        unsigned long freehigh;                       /* Available high memory size */
        unsigned int mem_unit;                        /* Memory unit size in bytes */
        char _f[20 - 2 * sizeof(long) - sizeof(int)]; /* Padding to 64 bytes */
    };
    struct sysinfo *sysinfostruct = (struct sysinfo *)_info;
    sysinfostruct->uptime = time::getUptimeSeconds();
    sysinfostruct->totalram = memorymap::total_ram;
    sysinfostruct->freeram = mm::phys::phys_get_free_blocks() * ARCH_PAGE_SIZE;
    sysinfostruct->sharedram = 0;
    sysinfostruct->bufferram = 0;
    sysinfostruct->totalswap = 0;
    sysinfostruct->freeswap = 0;
    sysinfostruct->procs = multitasking::getProcessCount();
    sysinfostruct->totalhigh = 0;
    sysinfostruct->freehigh = 0;
    sysinfostruct->mem_unit = 1;
    return 0;
}

// sys_clone defined in multitasking.cpp

uint32_t sys_uname(isr::registers *, int *syscall_ret, uint32_t, uint32_t _old_utsname, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    struct sys_uname_utsname {
        char sysname[65];
        char nodename[65];
        char release[65];
        char version[65];
        char machine[65];
    };
    struct sys_uname_utsname *unamestruct = (struct sys_uname_utsname *)_old_utsname;
    memcpy(unamestruct->sysname, "shitOS", 7);
    memcpy(unamestruct->nodename, "puter", 6);
    memcpy(unamestruct->release, CONFIG_KVERSION, strlen(CONFIG_KVERSION) + 1);
    memcpy(unamestruct->version, __DATE__ " " __TIME__, strlen(__DATE__ " " __TIME__) + 1);
    memcpy(unamestruct->machine, CONFIG_ARCH, 4);
    return 0;
}

uint32_t sys_modify_ldt(isr::registers *, int *syscall_ret, uint32_t, uint32_t _func, uint32_t _ptr, uint32_t bytecount, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    struct user_desc {
        unsigned int entry_number;
        unsigned int base_addr;
        unsigned int limit;
        unsigned int seg_32bit : 1;
        unsigned int contents : 2;
        unsigned int read_exec_only : 1;
        unsigned int limit_in_pages : 1;
        unsigned int seg_not_present : 1;
        unsigned int useable : 1;
    };
    int func = (int)_func;
    struct user_desc *ptr = (struct user_desc *)_ptr;
    LOG_INSANE("syscall: sys_modify_ldt");
    DEBUG_PRINTF("func: %d ptr: 0x%p bytecount: %u\n", func, ptr, bytecount);
    if (sizeof(user_desc) != bytecount) {
        LOG_INSANE("sizeof(user_desc) != bytecount");
        return -1;
    }
    DEBUG_PRINTF("entry_number: %u base_addr: 0x%p limit: %u seg_32bit: %u contents: %u read_exec_only: %u limit_in_pages: %u seg_not_present: %u useable: %u\n",
                 ptr->entry_number,
                 ptr->base_addr,
                 ptr->limit,
                 ptr->seg_32bit,
                 ptr->contents,
                 ptr->read_exec_only,
                 ptr->limit_in_pages,
                 ptr->seg_not_present,
                 ptr->useable);
    return -ENOSYS; // unimplemented
}

uint32_t sys_mprotect(isr::registers *, int *syscall_ret, uint32_t, uint32_t addr, uint32_t length, uint32_t prot, uint32_t flags, uint32_t _fd, uint32_t pgoffset) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_mprotect");
    return 0; // sure, worked flawlessly
}

uint32_t sys_writev(isr::registers *, int *syscall_ret, uint32_t, uint32_t fd, uint32_t _iov, uint32_t iovcnt, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;

    DEBUG_PRINTF("syscall: sys_writev\n");

    struct iovec {
        void *iov_base; /* Starting address */
        size_t iov_len; /* Number of bytes to transfer */
    };
    struct iovec *iov = (struct iovec *)_iov;

    uint32_t written = 0;

    for (int i = 0; i < (int)iovcnt; i++) {
        char *string = (char *)__builtin_alloca((iov->iov_len + 1));
        string[iov->iov_len] = '\0';
        memcpy(string, iov->iov_base, iov->iov_len);
        printf("%s", string);
        written += iov->iov_len;
        iov++;
    }

    return written; // return number of written bytes
}

uint32_t sys_rt_sigprocmask(isr::registers *, int *syscall_ret, uint32_t, uint32_t _how, uint32_t _set, uint32_t _oset, uint32_t sigsetsize, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_rt_sigprocmask");
    return 0;
}

uint32_t sys_getcwd(isr::registers *, int *syscall_ret, uint32_t, uint32_t _buf, uint32_t size, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    char *buf = (char *)_buf;
    LOG_INSANE("syscall: sys_getcwd");
    if (size < 11) {
        return 0;
    }
    memcpy(buf, "/home/user", 11);
    return 11;
}

uint32_t sys_mmap2(isr::registers *, int *syscall_ret, uint32_t, uint32_t addr, uint32_t length, uint32_t prot, uint32_t flags, uint32_t _fd, uint32_t pgoffset) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_mmap2 -- unstable");
    int fd = (int)_fd;

    void *alloc_adr = (void *)0xB69000;
    DEBUG_PRINTF("mmap2: program wants %u bytes\n", length);

    int pages = (length / ARCH_PAGE_SIZE);
    if (length % ARCH_PAGE_SIZE != 0) {
        pages += 1;
    }
    multitasking::x86_process *process = multitasking::getCurrentProcess();
    process->pages.push_back({.phys_base = (uint32_t)mm::phys::phys_malloc(pages), .virt_base = (uint32_t)alloc_adr, .pages = pages, .type = multitasking::process_pagerange::range_type::UNKNOWN});
    multitasking::setPageRange(&process->pages);

    return (uint32_t)alloc_adr;
}

uint32_t sys_stat64(isr::registers *, int *syscall_ret, uint32_t, uint32_t _filename, uint32_t _statbuf, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    const char *filename = (const char *)_filename;
    LOG_INSANE("syscall: sys_stat64");
    printf("%s\n", filename);
    return -ENOSYS; // TODO: fix up return value
}

uint32_t sys_getuid32(isr::registers *, int *syscall_ret, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_getuid32");
    return 0; // with the current state of the system we are always root
}

uint32_t sys_fcntl64(isr::registers *, int *syscall_ret, uint32_t, uint32_t _fd, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_fcntl64");
    int fd = (int)_fd;
    DEBUG_PRINTF("fd: %d\n", fd);
    return -ENOSYS;
}

uint32_t sys_futex(isr::registers *, int *syscall_ret, uint32_t, uint32_t _uaddr, uint32_t _op, uint32_t val, uint32_t _timeout, uint32_t _uaddr2, uint32_t val3) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_futex");
    uint32_t *uaddr = (uint32_t *)_uaddr;
    int op = (int)_op;
    uint64_t *timeout = (uint64_t *)_timeout;
    uint32_t *uaddr2 = (uint32_t *)_uaddr2;

    DEBUG_PRINTF("futex: uaddr -> 0x%p op -> %d val -> 0x%p timeout -> 0x%p uaddr2 -> 0x%p val3 -> 0x%p\n", _uaddr, op, val, _timeout, _uaddr2, val3);
    op &= 0xF;
    DEBUG_PRINTF("futex2: uaddr -> 0x%p op -> %d val -> 0x%p timeout -> 0x%p uaddr2 -> 0x%p val3 -> 0x%p\n", _uaddr, op, val, _timeout, _uaddr2, val3);
    if (op == 1) { // FUTEX_WAKE
        return 1;  // :troll:
    }

    return -ENOSYS;
}

uint32_t sys_set_thread_area(isr::registers *, int *syscall_ret, uint32_t, uint32_t _usr_desc, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_set_thread_area");
    struct user_desc {
        unsigned int entry_number;
        unsigned int base_addr;
        unsigned int limit;
        unsigned int seg_32bit : 1;
        unsigned int contents : 2;
        unsigned int read_exec_only : 1;
        unsigned int limit_in_pages : 1;
        unsigned int seg_not_present : 1;
        unsigned int useable : 1;
        unsigned int lm : 1;
    };
    struct user_desc *user_desc_p = (struct user_desc *)_usr_desc;
    if (user_desc_p->entry_number != (unsigned int)-1) {
        LOG_INSANE("sys_set_thread_area does not support entry_number != -1");
        return -ENOSYS;
    }
    if (user_desc_p->contents != 0) {
        LOG_INSANE("sys_set_thread_area does not support contents != 0");
        return -ENOSYS;
    }
    if (!user_desc_p->seg_32bit) {
        LOG_INSANE("sys_set_thread_area does not support seg_32bit = false");
        return -ENOSYS;
    }
    if (user_desc_p->base_addr == 0) {
        LOG_INSANE("sys_set_thread_area does not support base_addr = 0");
        return -ENOSYS;
    }

    DEBUG_PRINTF("entry_number: %u base_addr: 0x%p limit: %u seg_32bit: %u contents: %u read_exec_only: %u limit_in_pages: %u seg_not_present: %u useable: %u\n",
                 user_desc_p->entry_number,
                 user_desc_p->base_addr,
                 user_desc_p->limit,
                 user_desc_p->seg_32bit,
                 user_desc_p->contents,
                 user_desc_p->read_exec_only,
                 user_desc_p->limit_in_pages,
                 user_desc_p->seg_not_present,
                 user_desc_p->useable);

    // load data for tls
    multitasking::x86_process *proc = multitasking::getCurrentProcess();
    if (proc->tlsinfo.tlsdata != nullptr) {
        memset((uint8_t *)user_desc_p->base_addr - proc->tlsinfo.tls_size, 0, proc->tlsinfo.tls_size);
        memcpy((uint8_t *)user_desc_p->base_addr - proc->tlsinfo.tls_size, proc->tlsinfo.tlsdata, proc->tlsinfo.tlsdata_size);
    }

    uint8_t access = 0;
    uint8_t flags = 0;
    if (user_desc_p->limit_in_pages) {
        flags |= 0x8;
    }
    flags |= 0x4; // 32-bit

    access |= 0x02; // GDT_ACCESS_CODE_READABLE / GDT_ACCESS_DATA_WRITEABLE
    access |= 0x80; // GDT_ACCESS_PRESENT
    access |= 0x60; // GDT_ACCESS_RING3
    access |= 0x10; // GDT_ACCESS_DATA_SEGMENT

    gdt::set_tls_entry(user_desc_p->base_addr, user_desc_p->limit, access, flags);

    user_desc_p->entry_number = 6;

    return 0;
}

uint32_t sys_set_tid_address(isr::registers *, int *syscall_ret, uint32_t, uint32_t tidptr_u, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_set_tid_address");
    int *tidptr = (int *)tidptr_u;
    // TODO: implement together with clone
    return 69;
    return multitasking::getCurrentProcess()->tgid;
}

uint32_t sys_set_robust_list(isr::registers *, int *syscall_ret, uint32_t, uint32_t _robust_list_head, uint32_t _len, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_set_robust_list");
    struct robust_list {
        struct robust_list *next;
    };
    struct robust_list_head {
        struct robust_list list;
        long futex_offset;
        struct robust_list *list_op_pending;
    };
    struct robust_list_head *head = (struct robust_list_head *)_robust_list_head;
    size_t len = (size_t)_len;
    if (len != sizeof(*head)) {
        return -EINVAL;
    }

    return 0; // sure, it worked :troll:
}

uint32_t sys_rseq(isr::registers *, int *syscall_ret, uint32_t, uint32_t rseq_u, uint32_t rseq_len, uint32_t flags_u, uint32_t sig, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_rseq");
    return -ENOSYS;
    // https://lwn.net/Articles/774098/
    // https://lwn.net/Articles/883104/
    // linux: include/uapi/linux/rseq.h
    struct rseq {
        uint32_t cpu_id_start;
        uint32_t cpu_id;
        uint64_t rseq_cs;
        uint32_t flags;
        uint32_t node_id;
        uint32_t mm_cid;
    } __attribute__((aligned(4 * sizeof(uint64_t))));
    struct rseq_cs {
        uint32_t version;
        uint64_t start_ip;
        uint64_t post_commit_offset;
        uint64_t abort_ip;
    } __attribute__((aligned(4 * sizeof(uint64_t))));
    struct rseq *rseq = (struct rseq *)rseq_u;
    struct rseq_cs *rseq_cs = (struct rseq_cs *)rseq->rseq_cs;
    if (rseq_cs != nullptr) {
        DEBUG_PRINTF_INSANE("struct rseq_cs: version -> 0x%p start_ip(lower 32) -> 0x%p post_commit_offset(lower 32) -> 0x%p abort_ip(lower 32) -> 0x%p\n",
                            rseq_cs->version,
                            (uint32_t)(rseq_cs->start_ip & 0xFFFFFFFF),
                            (uint32_t)(rseq_cs->post_commit_offset & 0xFFFFFFFF),
                            (uint32_t)(rseq_cs->abort_ip & 0xFFFFFFFF));
    }
    DEBUG_PRINTF_INSANE("struct rseq: cpu_id_start->%u cpu_id->%u rseq_cs(only first u32)->%u rseq_cs(last u32)->%u flags->%u\n",
                        rseq->cpu_id_start,
                        rseq->cpu_id,
                        (uint32_t)rseq->rseq_cs,
                        (uint32_t)(rseq->rseq_cs >> 32),
                        rseq->flags);
    int flags = flags_u;
    DEBUG_PRINTF_INSANE("rseq_len: 0x%p, flags: %d sig: 0x%p\n", rseq_len, flags, sig);
    DEBUG_PRINTF_INSANE("rseq sizeof %u\n", sizeof(struct rseq));
    if (flags == 0) { // registration
        DEBUG_PRINTF("registration\n");
        rseq->cpu_id_start = 0;
        rseq->cpu_id = 0;
        rseq->node_id = 0;
        rseq->mm_cid = 0;
    }
    return 0; // sure, it worked fine
}
