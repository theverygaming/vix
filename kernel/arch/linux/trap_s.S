#include <vix/arch/monitor.h>
.section .text
.global trap_handler
trap_handler:
    mov $MONITOR_CALL, %rax
    mov $MONITOR_CALL_TRAPRET, %rdi
    syscall
