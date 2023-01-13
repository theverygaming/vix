#include <arch/drivers/timer.h>
#include <stdio.h>
#include <types.h>

// https://tc.gts3.org/cs3210/2020/spring/r/aarch64-generic-timer.pdf

void drivers::timer::init() {
    uint64_t a = (1 << 0);
    asm volatile("msr cntp_ctl_el0, %0" ::"r"(a) :);
    a = 1232132;
    asm volatile("msr cntp_cval_el0, %0" ::"r"(a) :);
    a = 112322;
    asm volatile("msr cntp_tval_el0, %0" ::"r"(a) :);
}
