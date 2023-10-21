#include <abi/linux/calls.h>
#include <abi/linux/errno.h>
#include <drivers/keyboard.h>
#include <kprintf.h>
#include <sched.h>
#include <stdio.h>
#include <types.h>

struct read_ev_info {
    uint32_t fd;
    char *buf;
    int read;
    int read_max;
};

static bool key_listener(void *ctx, const char &c) {
    struct read_ev_info *info = (struct read_ev_info *)ctx;
    info->buf[info->read] = c;
    info->read++;
    if (info->read >= info->read_max || c == '\n') {
        return true;
    }
    return false;
}

__DEF_LINUX_SYSCALL(sys_read) {
    int fd = (int)sysarg0;
    uint8_t *buf = (uint8_t *)sysarg1;
    size_t count = (size_t)sysarg2;

    if (count == 0) {
        return count;
    }

    if (fd != 0) {
        return -EBADF;
    }

    struct read_ev_info *info = (struct read_ev_info *)mm::kmalloc(sizeof(read_ev_info));
    drivers::keyboard::events.register_listener(key_listener, info);

    *info = {
        .fd = fd,
        .buf = (char *)buf,
        .read = 0,
        .read_max = count,
    };

    int read;
    while (true) {
        if (info->read >= info->read_max || (info->read > 0 && info->buf[info->read - 1] == '\n')) {
            read = info->read;
            mm::kfree(info);
            break;
        }
        sched::yield();
    }

    return read;
}
