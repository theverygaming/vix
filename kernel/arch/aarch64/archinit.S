.global _initcpulock

.global _init_cpu_lock
_init_cpu_lock:
.lock:
    mov     x5, 1
    ldr     x2, =_initcpulock
    ldxr    x3, [x2]
    stxr    w4, x5, [x2]
    cbnz    w4, .lock
    dmb ish
    cbnz    x3, .lock
    ret

.global _init_cpu_unlock
_init_cpu_unlock:
    ldr     x2, =_initcpulock
    stlr    xzr, [x2]
    ret
