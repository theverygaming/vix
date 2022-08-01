#include "syscalls.h"
#include "debug.h"
#include "drivers/keyboard.h"
#include "memalloc.h"
#include "multitasking.h"
#include "paging.h"
#include "stdio.h"
#include "stdlib.h"

uint32_t sys_exit(uint32_t, uint32_t exit_code, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    DEBUG_PRINTF("syscall: sys_exit code: %d\n", exit_code);
    multitasking::killCurrentProcess();
}

uint32_t sys_fork(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    DEBUG_PRINTF("syscall: sys_fork\n");
    multitasking::process *newprocess = multitasking::fork_current_process();
    if (newprocess) {
        newprocess->registerContext.eax = 0;
        return newprocess->pid;
    } else {
        return -1; // TODO: correct return value
    }
}

uint32_t sys_read(uint32_t, uint32_t fd, uint32_t _buf, uint32_t count, uint32_t, uint32_t, uint32_t) {
    char *buf = (char *)_buf;
    DEBUG_PRINTF("syscall: sys_read\n");

    int bufStart = drivers::keyboard::bufferlocation;
    while (drivers::keyboard::bufferlocation - bufStart < (int)count) {
        drivers::keyboard::manualRead();
        if (drivers::keyboard::buffer[drivers::keyboard::bufferlocation] == '\n') {
            break;
        }
    }
    uint32_t readCharacters = drivers::keyboard::bufferlocation - bufStart;

    drivers::keyboard::bufferlocation = -1;
    s_memcpy(buf, &drivers::keyboard::buffer[bufStart + 1], readCharacters);
    return readCharacters;
}

uint32_t sys_write(uint32_t, uint32_t fd, uint32_t _buf, uint32_t count, uint32_t, uint32_t, uint32_t) {
    char *buf = (char *)_buf;

    DEBUG_PRINTF("syscall: sys_write count: %u\n", count);
    if (!paging::is_readable(buf + count)) {
        return -1; // TODO: proper return value
    }
    char string[count + 1];
    string[count] = '\0';
    memcpy(string, buf, count);
    printf("%s", string);
    return count; // return number of written bytes
}

uint32_t sys_waitpid(uint32_t, uint32_t pid, uint32_t _stat_addr, uint32_t _options, uint32_t, uint32_t, uint32_t) {
    int *stat_addr = (int *)_stat_addr;
    int options = (uint32_t)_options;

    DEBUG_PRINTF("syscall: sys_waitpid - PID: %d\n", pid);

    multitasking::context *current_context = (multitasking::context *)(KERNEL_VIRT_ADDRESS + REGISTER_STORE_OFFSET);
    multitasking::context tempContextStore;
    memcpy((char *)&tempContextStore, (char *)current_context, sizeof(multitasking::context));

    asm("sti"); // TODO: figure out why this crashes when kernel memory runs out

    multitasking::waitForProcess(pid);

    asm("cli");
    memcpy((char *)current_context, (char *)&tempContextStore, sizeof(multitasking::context));
}

uint32_t sys_execve(uint32_t, uint32_t _filename, uint32_t _argv, uint32_t _envp, uint32_t, uint32_t, uint32_t) {
    const char *filename = (const char *)_filename;
    const char *const *argv = (const char *const *)_argv;
    const char *const *envp = (const char *const *)_envp;
    DEBUG_PRINTF("syscall: sys_execve\n");
    printf("%s\n", filename);
}

uint32_t sys_mmap(uint32_t, uint32_t mmap_struct_ptr, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    DEBUG_PRINTF("syscall: sys_mmap !! unstable\n");
    typedef struct {
        void *start;
        uint32_t length;
        int prot;
        int flags;
        int fd;
        int offset;
    } mmap_args_t;
    mmap_args_t *args = (mmap_args_t *)mmap_struct_ptr;

    void *alloc_adr = (void *)0xB69420;
    printf("mmap: program wants %u bytes\n", args->length);

    int pages = (args->length / 4096) + 1;
    void *map = memalloc::page::phys_malloc(pages);
    for (int i = 0; i < pages; i++) {
        paging::map_page(map + (i * 4096), alloc_adr + (i * 4096));
    }
    multitasking::refresh_current_process_pagerange();

    return (uint32_t)alloc_adr;
}

uint32_t sys_stat64(uint32_t, uint32_t _filename, uint32_t _statbuf, uint32_t, uint32_t, uint32_t, uint32_t) {
    const char* filename = (const char*)_filename;
    DEBUG_PRINTF("syscall: sys_stat64\n");
    printf("%s\n", filename);
}

uint32_t sys_getuid32(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
    DEBUG_PRINTF("syscall: sys_getuid32\n");
    return 0; // with the current state of the system we are always root
}
