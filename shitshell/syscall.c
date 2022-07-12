#include "syscall.h"

extern uint32_t syscall(void* syscall_num, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6);

void sys_write(uint32_t fd, const char* buf, size_t count) {
    syscall(4, fd, buf, count, 0, 0, 0);
}

uint32_t sys_read(uint32_t fd, char* buf, size_t count) {
    return syscall(3, fd, buf, count, 0, 0, 0);
}

void sys_waitpid(pid_t pid, int* start_addr, int options) {
    syscall(7, pid, start_addr, options, 0, 0, 0);
}

pid_t sys_fork() {
    return syscall(2, 0, 0, 0, 0, 0, 0);
}

void sys_execve(const char* filename, char** argv, const char *const *envp) {
    syscall(11, filename, argv, envp, 0, 0, 0);
}