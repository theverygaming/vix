#include <vix/abi/vix/calls.h>
#include <vix/kprintf.h>
#include <vix/sched.h>

__DEF_VIX_SYSCALL(sys_exit) {
    int status = (int)sysarg0;
    kprintf(KP_INFO, "TID %d dying status: %d\n", sched::mythread()->tid, status);
    sched::die();
    return 0;
}
