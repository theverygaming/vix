#include "syscall.h"

extern uint32_t syscall(void* syscall_num, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6);

void sys_write(uint32_t fd, const char* buf, size_t count) {
    syscall(4, fd, (void*)buf, (void*)count, 0, 0, 0);
}

uint32_t sys_read(uint32_t fd, char* buf, size_t count) {
    return syscall(3, fd, (void*)buf, (void*)count, 0, 0, 0);
}

