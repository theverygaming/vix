#include <vix/abi/linux/calls.h>
#include <vix/abi/linux/errno.h>
#include <vix/drivers/keyboard.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/sched.h>
#include <vix/stdio.h>
#include <vix/types.h>

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

    // we allocate a second buffer as when they key_listener gets called
    // we may not be in the same thread which means memory mappings are different!
    char *read_buf = (char *)mm::kmalloc(count);

    struct read_ev_info *info = (struct read_ev_info *)mm::kmalloc(sizeof(read_ev_info));
    drivers::keyboard::events.register_listener(key_listener, info);

    *info = {
        .fd = fd,
        .buf = (char *)read_buf,
        .read = 0,
        .read_max = count,
    };

    int read;
    while (true) {
        if (info->read >= info->read_max || (info->read > 0 && info->buf[info->read - 1] == '\n')) {
            read = info->read;
            // copy out buffer to user memory
            memcpy(buf, read_buf, read);
            mm::kfree(read_buf);
            mm::kfree(info);
            break;
        }
        sched::yield();
    }

    return read;
}
