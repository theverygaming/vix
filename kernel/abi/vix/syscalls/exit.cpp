#include <abi/vix/calls.h>
#include <kprintf.h>
#include <sched.h>

__DEF_VIX_SYSCALL(sys_exit) {
    int status = (int)sysarg0;
    kprintf(KP_INFO, "PID %d dying status: %d\n", sched::mypid(), status);
    sched::die();
    return 0;
}
