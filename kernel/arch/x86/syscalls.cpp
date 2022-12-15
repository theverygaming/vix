#include <arch.h>
#include <arch/drivers/keyboard.h>
#include <arch/elf.h>
#include <arch/errno.h>
#include <arch/generic/memory.h>
#include <arch/memorymap.h>
#include <arch/multitasking.h>
#include <arch/paging.h>
#include <arch/syscalls.h>
#include <cppstd/string.h>
#include <debug.h>
#include <fs/vfs.h>
#include <generated/autoconf.h>
#include <log.h>
#include <memory_alloc/memalloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint32_t sys_exit(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t exit_code, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 0;
    DEBUG_PRINTF("syscall: sys_exit code: %d\n", exit_code);
    multitasking::killCurrentProcess(regs);
    return 0;
}

uint32_t sys_fork(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_fork\n");
    multitasking::x86_process *newprocess = multitasking::fork_current_process(regs);
    if (newprocess) {
        newprocess->registerContext.eax = 0;
        return newprocess->pid;
    } else {
        return -1; // TODO: correct return value
    }
}

uint32_t sys_read(isr::registers *, int *syscall_ret, uint32_t, uint32_t fd, uint32_t _buf, uint32_t count, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    char *buf = (char *)_buf;
    LOG_INSANE("syscall: sys_read\n");

    int bufStart = drivers::keyboard::bufferlocation;
    while (drivers::keyboard::bufferlocation - bufStart < (int)count) {
        drivers::keyboard::manualRead();
        if (drivers::keyboard::buffer[drivers::keyboard::bufferlocation] == '\n') {
            break;
        }
    }
    uint32_t readCharacters = drivers::keyboard::bufferlocation - bufStart;

    drivers::keyboard::bufferlocation = -1;
    memcpy(buf, &drivers::keyboard::buffer[bufStart + 1], readCharacters);
    return readCharacters;
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

uint32_t sys_waitpid(isr::registers *, int *syscall_ret, uint32_t, uint32_t pid, uint32_t _stat_addr, uint32_t _options, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    // int *stat_addr = (int *)_stat_addr;
    // int options = (uint32_t)_options;

    DEBUG_PRINTF("syscall: sys_waitpid - PID: %d\n", pid);

    /*multitasking::context *current_context = (multitasking::context *)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET);
    multitasking::context tempContextStore;
    memcpy(&tempContextStore, current_context, sizeof(multitasking::context));

    asm volatile("sti"); // TODO: figure out why this crashes when kernel memory runs out

    multitasking::waitForProcess(pid);

    asm volatile("cli");
    memcpy(current_context, &tempContextStore, sizeof(multitasking::context));*/
    // KERNEL_PANIC("sys_waitpid unimplemented");
    return 0; // TODO: fix return value
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
        elf::load_program(elfptr, &args, true, multitasking::getCurrentProcess()->pid, regs);
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
    uintptr_t break_adr = process->brk_start + (brk_pages * ARCH_PAGE_SIZE);

    if (_brk == 0) {
        return break_adr;
    }

    if (break_adr >= _brk) {
        return _brk; // we never actually deallocate :troll:
    } else {
        uint32_t newbrk = _brk;
        if (newbrk % ARCH_PAGE_SIZE != 0) { // align brk with page
            newbrk += ARCH_PAGE_SIZE - (newbrk % ARCH_PAGE_SIZE);
        }
        size_t needed_pages = (newbrk - break_adr) / ARCH_PAGE_SIZE;
        process->pages.push_back({.phys_base = (uint32_t)memalloc::page::phys_malloc(needed_pages),
                                  .virt_base = process->brk_start + (brk_pages * ARCH_PAGE_SIZE),
                                  .pages = needed_pages,
                                  .type = multitasking::process_pagerange::range_type::BREAK});
        multitasking::setPageRange(&process->pages);
    }
    return _brk;
}

uint32_t sys_mmap(isr::registers *, int *syscall_ret, uint32_t, uint32_t mmap_struct_ptr, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_mmap -- unstable\n");
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
    uint8_t *map = (uint8_t *)memalloc::page::phys_malloc(pages);
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
    sysinfostruct->freeram = memalloc::page::phys_get_free_blocks() * ARCH_PAGE_SIZE;
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

uint32_t modify_ldt(isr::registers *, int *syscall_ret, uint32_t, uint32_t _func, uint32_t _ptr, uint32_t bytecount, uint32_t, uint32_t, uint32_t) {
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
    LOG_INSANE("syscall: modify_ldt\n");
    DEBUG_PRINTF("func: %d ptr: 0x%p bytecount: %u\n", func, ptr, bytecount);
    if (sizeof(user_desc) != bytecount) {
        LOG_INSANE("sizeof(user_desc) != bytecount\n");
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

uint32_t sys_getcwd(isr::registers *, int *syscall_ret, uint32_t, uint32_t _buf, uint32_t size, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    char *buf = (char *)_buf;
    LOG_INSANE("syscall: sys_getcwd\n");
    if (size < 11) {
        return 0;
    }
    memcpy(buf, "/home/user", 11);
    return 11;
}

uint32_t sys_stat64(isr::registers *, int *syscall_ret, uint32_t, uint32_t _filename, uint32_t _statbuf, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    const char *filename = (const char *)_filename;
    LOG_INSANE("syscall: sys_stat64\n");
    printf("%s\n", filename);
    return -ENOSYS; // TODO: fix up return value
}

uint32_t sys_getuid32(isr::registers *, int *syscall_ret, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: sys_getuid32\n");
    return 0; // with the current state of the system we are always root
}

uint32_t set_thread_area(isr::registers *, int *syscall_ret, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 1;
    LOG_INSANE("syscall: set_thread_area\n");
    return -ENOSYS; // unimplemented
}
