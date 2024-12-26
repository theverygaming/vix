#include <vix/abi/vix/calls.h>
#include <vix/kprintf.h>
#include <vix/sched.h>

__DEF_VIX_SYSCALL(sys_exit) {
    int status = (int)sysarg0;
    kprintf(KP_INFO, "PID %d dying status: %d\n", sched::mytask()->pid, status);
    sched::die();
    return 0;
}
