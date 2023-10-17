#include <arch/drivers/ps2.h>
#include <arch/multitasking.h>
#include <arch/syscalls_32.h>
#include <debug.h>
#include <mm/kmalloc.h>
#include <scheduler.h>
#include <stdlib.h>
#include <types.h>

struct read_resume_info {
    pid_t tid;
    uint32_t fd;
    char *buf;
    int read;
    int read_max;
};

static bool key_listener(void *ctx, const char &c) {
    struct read_resume_info *info = (struct read_resume_info *)ctx;
    multitasking::x86_process *proc = multitasking::get_tid(info->tid);
    assertm(proc != nullptr, "why it dead??????");

    multitasking::unsetPageRange(&multitasking::getCurrentProcess()->pages);
    multitasking::setPageRange(&proc->pages);
    info->buf[info->read] = c;
    info->read++;
    multitasking::unsetPageRange(&proc->pages);
    multitasking::setPageRange(&multitasking::getCurrentProcess()->pages);

    if (info->read >= info->read_max || c == '\n') {
        proc->state = schedulers::generic_process::state::RUNNABLE;
        proc->reg_ctx.eax = info->read;
        DEBUG_PRINTF("sys_read return: %d\n", info->read);
        mm::kfree(info);
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

    *syscall_ret = 1;
    const char *str = "sysinfo\n";
    if(count > strlen(str)) {
        count = strlen(str);
    }
    memcpy((void *)_buf, str, count);
    return count;

    /*
    struct read_resume_info *info = (struct read_resume_info *)mm::kmalloc(sizeof(read_resume_info));
    multitasking::x86_process *current = multitasking::getCurrentProcess();
    current->state = schedulers::generic_process::state::UNINTERRUPTIBLE_SLEEP;
    drivers::keyboard::events.register_listener(key_listener, info);

    *info = {
        .tid = current->tgid,
        .fd = fd,
        .buf = (char *)_buf,
        .read = 0,
        .read_max = count,
    };

    multitasking::reschedule(regs);
    return 0;*/
}
