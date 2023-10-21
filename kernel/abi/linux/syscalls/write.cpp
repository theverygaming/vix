#include <abi/linux/calls.h>
#include <abi/linux/errno.h>
#include <kprintf.h>
#include <sched.h>
#include <stdio.h>
#include <types.h>

__DEF_LINUX_SYSCALL(sys_write) {
    int fd = (int)sysarg0;
    uint8_t *buf = (uint8_t *)sysarg1;
    size_t count = (size_t)sysarg2;

    if (!(fd == 1 || fd == 2)) {
        return -EBADF;
    }

    for (size_t i = 0; i < count; i++) {
        putc((char)buf[i]);
    }
    return count;
}
