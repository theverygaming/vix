#include <arch/drivers/ps2.h>
#include <arch/multitasking.h>
#include <arch/syscalls_32.h>
#include <debug.h>
#include <mm/kmalloc.h>
#include <sched.h>
#include <scheduler.h>
#include <stdlib.h>
#include <types.h>

struct read_resume_info {
    uint32_t fd;
    char *buf;
    int read;
    int read_max;
};

static bool key_listener(void *ctx, const char &c) {
    struct read_resume_info *info = (struct read_resume_info *)ctx;
    //multitasking::unsetPageRange(&multitasking::getCurrentProcess()->pages);
    //multitasking::setPageRange(&proc->pages);
    info->buf[info->read] = c;
    info->read++;
    //multitasking::unsetPageRange(&proc->pages);
    //multitasking::setPageRange(&multitasking::getCurrentProcess()->pages);

    if (info->read >= info->read_max || c == '\n') {
        //proc->state = schedulers::generic_process::state::RUNNABLE;
        //proc->reg_ctx.eax = info->read;
        //DEBUG_PRINTF("sys_read return: %d\n", info->read);
        return true;
    }
    return false;
}

uint32_t sys_read(struct arch::full_ctx *regs, int *syscall_ret, uint32_t, uint32_t fd, uint32_t _buf, uint32_t count, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 0;
    DEBUG_PRINTF("syscall: sys_read\n");
    if (count == 0) {
        *syscall_ret = 1;
        return 0;
    }

    struct read_resume_info *info = (struct read_resume_info *)mm::kmalloc(sizeof(read_resume_info));
    drivers::keyboard::events.register_listener(key_listener, info);

    *info = {
        .fd = fd,
        .buf = (char *)_buf,
        .read = 0,
        .read_max = count,
    };

    uint32_t read;
    while (true) {
        if (info->read >= info->read_max || (info->read > 0 && info->buf[info->read - 1] == '\n')) {
            read = info->read;
            mm::kfree(info);
            break;
        }
        sched::yield();
    }

    DEBUG_PRINTF("sys_read return: %d\n", read);
    return read;
}
