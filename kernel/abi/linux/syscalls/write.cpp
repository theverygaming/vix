#include <vix/abi/linux/calls.h>
#include <vix/abi/linux/errno.h>
#include <vix/kprintf.h>
#include <vix/sched.h>
#include <vix/stdio.h>
#include <vix/types.h>

__DEF_LINUX_SYSCALL(sys_write) {
    int fd = (int)sysarg0;
    uint8_t *buf = (uint8_t *)sysarg1;
    size_t count = (size_t)sysarg2;

    if (!(fd == 1 || fd == 2)) {
        return -LINUX_EBADF;
    }

    for (size_t i = 0; i < count; i++) {
        putc((char)buf[i]);
    }
    return count;
}
