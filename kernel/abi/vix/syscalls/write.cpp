#include <vix/abi/vix/calls.h>
#include <vix/abi/vix/errno.h>
#include <vix/stdio.h>
#include <vix/interrupts.h>

__DEF_VIX_SYSCALL(sys_write) {
    int fd = (int)sysarg0;
    uint8_t *buf = (uint8_t *)sysarg1;
    size_t count = (size_t)sysarg2;

    if (!(fd == 1 || fd == 2)) {
        return VIX_EBADF;
    }

    for (size_t i = 0; i < count; i++) {
        push_interrupt_disable(); // FIXME: putc should have proper locks!
        putc((char)buf[i]);
        pop_interrupt_disable();
    }
    return count;
}
