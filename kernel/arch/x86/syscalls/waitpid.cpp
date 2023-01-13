#include <arch/drivers/ps2.h>
#include <arch/multitasking.h>
#include <arch/syscalls.h>
#include <debug.h>
#include <errno.h>
#include <log.h>
#include <mm/kmalloc.h>
#include <scheduler.h>
#include <stdlib.h>
#include <types.h>

struct waitpid_resume_info {
    pid_t self_tid;
    pid_t tid;
};

static bool deth_listener(void *ctx, const pid_t *tid) {
    struct waitpid_resume_info *info = (struct waitpid_resume_info *)ctx;

    if (*tid != info->tid) {
        return false;
    }

    multitasking::x86_process *proc = multitasking::get_tid(info->self_tid);
    assertm(proc != nullptr, "why it ded??????");

    proc->state = schedulers::generic_process::state::RUNNABLE;
    proc->registerContext.eax = info->tid;
    DEBUG_PRINTF("sys_waitpid return: %d\n", info->tid);
    mm::kfree(info);
    return true;
}

uint32_t sys_waitpid(isr::registers *regs, int *syscall_ret, uint32_t, uint32_t pid, uint32_t _stat_addr, uint32_t _options, uint32_t, uint32_t, uint32_t) {
    *syscall_ret = 0;
    LOG_INSANE("syscall: sys_waitpid");
    DEBUG_PRINTF("pid: %d\n", pid);
    multitasking::x86_process *proc = multitasking::get_tid(pid);
    if (proc == nullptr) {
        *syscall_ret = 1;
        return -ECHILD;
    }

    struct waitpid_resume_info *info = (struct waitpid_resume_info *)mm::kmalloc(sizeof(waitpid_resume_info));
    multitasking::x86_process *current = multitasking::getCurrentProcess();
    current->state = schedulers::generic_process::state::UNINTERRUPTIBLE_SLEEP;
    multitasking::process_deth_events.register_listener(deth_listener, info);

    *info = {
        .self_tid = current->tgid,
        .tid = pid,
    };

    multitasking::reschedule(regs);
    return 0;
}
