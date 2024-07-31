#include <vix/abi/linux/calls.h>
#include <vix/kprintf.h>
#include <vix/sched.h>

__DEF_LINUX_SYSCALL(sys_exit) {
    int status = (int)sysarg0;
    kprintf(KP_INFO, "PID %d dying status: %d\n", sched::mypid(), status);
    sched::die();
    return 0;
}
