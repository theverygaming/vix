#include <vix/arch/monitor.h>
.section .text
.global trap_handler
trap_handler:
    add $(4 * 8), %rsp
    mov $MONITOR_CALL, %rax
    mov $MONITOR_CALL_TRAPRET, %rdi
    syscall
