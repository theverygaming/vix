#include <vix/debug.h>
#include <vix/kprintf.h>
#include <vix/arch/common/bootup.h>
#include <vix/config.h>
#include <vix/framebuffer.h>
#include <vix/fs/tarfs.h>
#include <vix/fs/vfs.h>
#include <vix/kernel.h>
#include <vix/mm/kheap.h>
#include <vix/mm/memmap.h>
#include <vix/panic.h>
#include <vix/stdio.h>
#include <vix/time.h>
#include <vix/arch/linux-syscalls.h>
#include <vix/arch/monitor.h>
#include <string.h>
#include <vix/arch/paging.h>

static void writeputs(const char *s, size_t n) {
    linux_write(1, s, n);
}

extern "C" uint8_t __text_start;
extern "C" uint8_t __text_end;
extern "C" uint8_t __rodata_start;
extern "C" uint8_t __rodata_end;
extern "C" uint8_t __data_start;
extern "C" uint8_t __data_end;
extern "C" uint8_t __bss_start;
extern "C" uint8_t __bss_end;

static void remap_code_helper(const char *name, int memfd, size_t memfd_bytes, uint8_t *raw, size_t *raw_offs, int prot, uint8_t *seg_start, uint8_t *seg_end, uint8_t *seg_next_start) {
    size_t seg_size = ALIGN_UP((size_t)(seg_end - seg_start), CONFIG_ARCH_PAGE_SIZE);
    uint8_t *seg_end_new = seg_start + seg_size;

    if (*raw_offs + seg_size > memfd_bytes) {
        kprintf(KP_ALERT, "not enough space for %s\n", name);
        linux_exit(1);
    }

    // copy data
    memcpy(&raw[*raw_offs], seg_start, seg_size);

    // map
    DEBUG_PRINTF("remap_code_helper mmap for %s 0x%p to 0x%p-0x%p\n", name, *raw_offs, seg_start, seg_start + seg_size);
    if ((ssize_t)linux_mmap(seg_start, seg_size, prot, LINUX_MAP_SHARED | LINUX_MAP_FIXED, memfd, *raw_offs) < 0) {
        kprintf(KP_ALERT, "mmap of %s failed", name);
        linux_exit(1);
    }

    (*raw_offs) += seg_size;

    // unmap previous segment
    if (seg_next_start != nullptr && seg_end_new < seg_next_start) {
        linux_munmap(seg_end_new, seg_next_start - seg_end_new);
        (*raw_offs) += seg_next_start - seg_end_new; // since we HHDM starting from kernel base, the gaps must be included in the file
    }
}

static size_t remap_code(int memfd, size_t memfd_bytes) {
    uint8_t *raw = (uint8_t *)linux_mmap(nullptr, memfd_bytes, LINUX_PROT_READ | LINUX_PROT_WRITE, LINUX_MAP_SHARED, memfd, 0);

    size_t raw_offs = 0;

    remap_code_helper(".text", memfd, memfd_bytes, raw, &raw_offs, LINUX_PROT_READ | LINUX_PROT_EXEC, &__text_start, &__text_end, &__rodata_start);
    remap_code_helper(".rodata", memfd, memfd_bytes, raw, &raw_offs, LINUX_PROT_READ, &__rodata_start, &__rodata_end, &__data_start);
    remap_code_helper(".data", memfd, memfd_bytes, raw, &raw_offs, LINUX_PROT_READ | LINUX_PROT_WRITE, &__data_start, &__data_end, &__bss_start);
    remap_code_helper(".bss", memfd, memfd_bytes, raw, &raw_offs, LINUX_PROT_READ | LINUX_PROT_WRITE, &__bss_start, &__bss_end, nullptr);

    linux_munmap(raw, memfd_bytes);
    return raw_offs;
}

static size_t remap_stack(int memfd, size_t memfd_bytes, size_t kernel_code_size) {
    int proc_maps_fd = linux_open("/proc/self/maps", LINUX_O_RDONLY, 0);
    if (proc_maps_fd < 0) {
        KERNEL_PANIC("got error %d", proc_maps_fd);
    }

    uint64_t sp;
    asm volatile("mov %%rsp, %0" : "=r"(sp));
    DEBUG_PRINTF("old SP: 0x%p\n", sp);

    struct linux_procmap_query query = {
        .size = sizeof(struct linux_procmap_query),
        .query_flags = 0, // in
        .query_addr = sp, // in
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
        KERNEL_PANIC("could not find stack map addr");
    }
    int err = linux_close(proc_maps_fd);
    if (err != 0) {
        KERNEL_PANIC("got error %d", err);
    }
    DEBUG_PRINTF("stack map: 0x%p-0x%p\n", query.vma_start, query.vma_end);

    uint64_t new_base = CONFIG_HHDM_VIRT_BASE + kernel_code_size;
    uint64_t new_size = CONFIG_ARCH_PAGE_SIZE * 8;
    uint64_t new_bottom = new_base + new_size;
    uint64_t old_top = query.vma_end;

    DEBUG_PRINTF("new_base: 0x%p new_size: 0x%p old_top: 0x%p new_bottom: 0x%p\n", new_base, new_size, old_top, new_bottom);

    asm volatile(
        // rcx: count
        // rcx: how many bytes of the old stack we want to copy at most
        "mov $" STRINGIFY(CONFIG_ARCH_PAGE_SIZE) ", %%rcx\n\t"
        // compute bytes from current sp to old stack top (actually bottom because it grows down) in rsi
        "mov %%rsp, %%rdi\n\t"
        "mov %[old_top], %%rsi\n\t"
        "sub %%rdi, %%rsi\n\t"
        // if rsi is smaller than rcx, move it to rcx
        "cmp %%rcx, %%rsi\n\t"
        "cmovb %%rsi, %%rcx\n\t"
        // rsi: src
        "mov %%rsp, %%rsi\n\t"
        // rdi: dst
        "mov %[new_bottom], %%rdi\n\t"
        "sub $" STRINGIFY(CONFIG_ARCH_PAGE_SIZE) ", %%rdi\n\t"
        // copy data from old to new stack
        "rep movsb\n\t"
        // correct rsp and rbp to the new stack (new rbp is computed via offset from old rsp)
        "mov %[new_bottom], %%rdi\n\t"
        "sub $" STRINGIFY(CONFIG_ARCH_PAGE_SIZE) ", %%rdi\n\t"
        "mov %%rsp, %%rsi\n\t"
        "mov %%rdi, %%rsp\n\t"
        "sub %%rdi, %%rsi\n\t"
        "sub %%rsi, %%rbp\n\t"
        :
        : [new_bottom] "r"(new_bottom), [old_top] "r"(old_top)
        : "rcx", "rsi", "rdi", "memory"
    );

    asm volatile("mov %%rsp, %0" : "=r"(sp));
    DEBUG_PRINTF("new SP: 0x%p\n", sp);

    err = linux_munmap((void *)query.vma_start, query.vma_end - query.vma_start);
    if (err != 0) {
        KERNEL_PANIC("munmap failed %d", err);
    }

    return new_size;
}

static void launch_monitor(int memfd, size_t memfd_bytes) {
    // create the monitor process
    linux_pid_t childpid = linux_fork();
    if (childpid) {
        monitor_entry(childpid, memfd, memfd_bytes);
        linux_kill(childpid, LINUX_SIGKILL);
        linux_exit(1); // this isn't really a normal exit path
    }
    // when the monitor process dies, we should too
    linux_prctl(LINUX_PR_SET_PDEATHSIG, LINUX_SIGKILL);

    // monitor shall trace us
    linux_ptrace(LINUX_PTRACE_TRACEME, 0, 0, 0);
    // wait til the monitor process wakes us back up (traceme needs a signal to work)
    //linux_kill(linux_getpid(), LINUX_SIGSTOP);
    linux_tgkill(linux_getpid(), linux_gettid(), LINUX_SIGSTOP);

    kprintf(KP_INFO, "kernel process started (PID %d)\n", linux_getpid());
}

int memfd;
size_t memfd_bytes;

extern "C" void trap_entry();

static void kernelinit() {
    stdio::set_puts_function(writeputs, true);

    // allocate main memory
    memfd = linux_memfd_create("vix memory", 0);
    memfd_bytes = 1 << 26 /* 64MiB */;
    linux_ftruncate(memfd, memfd_bytes);

    launch_monitor(memfd, memfd_bytes);

    monitor_set_trap_handler((uintptr_t)&trap_entry);

    // remap the stack and code _after_ branching off the monitor, because otherwise the stack and data would collide :P
    size_t kernel_code_size = remap_code(memfd, memfd_bytes);

    // map HHDM
    void *hhdm_base = (void *)(CONFIG_HHDM_VIRT_BASE + kernel_code_size);
    size_t hhdm_size = (memfd_bytes <= CONFIG_HHDM_SIZE ? memfd_bytes : CONFIG_HHDM_SIZE) - kernel_code_size;
    DEBUG_PRINTF("HHDM base: 0x%p size: %u\n", hhdm_base, hhdm_size);
    if (linux_mmap(hhdm_base, hhdm_size, LINUX_PROT_READ | LINUX_PROT_WRITE, LINUX_MAP_SHARED | LINUX_MAP_FIXED, memfd, kernel_code_size) != hhdm_base) {
        KERNEL_PANIC("failed to map HHDM");
    }

    size_t init_stack_size = remap_stack(memfd, memfd_bytes, kernel_code_size);

    struct mm::mem_map_entry r[] = {
        {
            .base = 0,
            .size = kernel_code_size,
            .type = mm::mem_map_entry::type_t::RESERVED,
        },
        {
            .base = kernel_code_size,
            .size = init_stack_size,
            .type = mm::mem_map_entry::type_t::RECLAIMABLE,
        },
        {
            .base = (kernel_code_size + init_stack_size),
            .size = memfd_bytes - (kernel_code_size + init_stack_size),
            .type = mm::mem_map_entry::type_t::RAM,
        },
    };
    mm::set_mem_map(r, sizeof(r) / sizeof(r[0]));

    kernelstart();
}

static void unmap_bullshit(long *auxv) {
    // find the vdso
    void *vdso_addr = nullptr;
    for (int i = 0; auxv[i] != 0; i += 2) {
        if (auxv[i] == LINUX_AT_SYSINFO_EHDR) {
            vdso_addr = (long *)auxv[i + 1];
            break;
        }
    }

    if (vdso_addr != nullptr) {
        // vvar (6 pages below vdso, 4 pages long)
        linux_munmap(((uint8_t *)vdso_addr) - (CONFIG_ARCH_PAGE_SIZE * 6), 4 * CONFIG_ARCH_PAGE_SIZE);
        // vvar (2 pages below vdso, 2 pages long)
        linux_munmap(((uint8_t *)vdso_addr) - (CONFIG_ARCH_PAGE_SIZE * 2), 2 * CONFIG_ARCH_PAGE_SIZE);
        // vdso
        linux_munmap(vdso_addr, 2 * CONFIG_ARCH_PAGE_SIZE);
    }
}

extern "C" void _kentry_c(int argc, char **argv, char **envp) {
    // figure out auxv
    char **envp1 = envp;
    while (*envp1 != nullptr) {
        envp1++;
    }
    long *auxv = (long *)(envp1 + 1);

    unmap_bullshit(auxv);

    kernelinit();
    while (true) {}
}

void arch::startup::stage2_startup() {
    paging_init();
    monitor_set_flags(MONITOR_FLAG_TIMER);
}

void arch::startup::stage3_startup() {}

void arch::startup::stage4_startup() {
    printf("Hello linux!\n");
    //*(volatile int*)1000 = 5; // cause segfault
    long ts = ((long (*)())0xffffffffff600400)();
    printf("vsyscall: %d\n", ts);
    for (volatile unsigned int i = 0; i < -1; i++) {
        volatile int x = linux_getpid(); 
        if ((i % 10000) == 0) {
            long ts = ((long (*)())0xffffffffff600400)();
            printf("vsyscall: %d\n", ts);
        }
        break;
    }
    time::bootupTime = time::getCurrentUnixTime();
}

void arch::startup::kthread0() {}
