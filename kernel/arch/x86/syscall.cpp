#include <arch/errno.h>
#include <arch/multitasking.h>
#include <arch/syscall.h>
#include <arch/syscalls.h>
#include <config.h>
#include <debug.h>

/* syscall arguments correspond to registers eax, ebx, ecx, edx, esi, edi and ebp */

// clang-format off
uint32_t (*syscall_table[440])(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = {
    &sys_dbg,
    &sys_exit,
    &sys_fork,
    &sys_read,
    &sys_write, /* 4   */
    &sys_open,
    &sys_close,
    &sys_waitpid,
    nullptr,
    nullptr, /* 9  */
    nullptr,
    &sys_execve,
    nullptr,
    &sys_time,
    nullptr, /* 14  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 19  */
    &sys_getpid,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 24  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 29  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 34  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 39  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 44  */
    &sys_brk,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 49  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &sys_ioctl, /* 54  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 59  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 64  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 69  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 74  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 79  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 84  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 89  */
    &sys_mmap,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 94  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 99 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 104 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 109 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 114 */
    nullptr,
    &sys_sysinfo,
    nullptr,
    nullptr,
    nullptr, /* 119 */
    &sys_clone,
    nullptr,
    &sys_uname,
    &sys_modify_ldt,
    nullptr, /* 124 */
    &sys_mprotect,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 129 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 134 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 139 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 144 */
    nullptr,
    &sys_writev,
    nullptr,
    nullptr,
    nullptr, /* 149 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 154 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 159 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 164 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 169 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 174 */
    &sys_rt_sigprocmask,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 179 */
    nullptr,
    nullptr,
    nullptr,
    &sys_getcwd,
    nullptr, /* 184 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 189 */
    nullptr,
    nullptr,
    &sys_mmap2,
    nullptr,
    nullptr, /* 194 */
    &sys_stat64,
    nullptr,
    nullptr,
    nullptr,
    &sys_getuid32, /* 199 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 204 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 209 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 214 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 219 */
    nullptr,
    &sys_fcntl64,
    nullptr,
    nullptr,
    nullptr, /* 224 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 229 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 234 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 239 */
    &sys_futex,
    nullptr,
    nullptr,
    &sys_set_thread_area,
    nullptr, /* 244 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 249 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 254 */
    nullptr,
    nullptr,
    nullptr,
    &sys_set_tid_address,
    nullptr, /* 259 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 264 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 269 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 274 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 279 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 284 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 289 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 294 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 299 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 304 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 309 */
    nullptr,
    &sys_set_robust_list,
    nullptr,
    nullptr,
    nullptr, /* 314 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 319 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 324 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 329 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 334 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 339 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 344 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 349 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 354 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 359 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 364 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 369 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 374 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 379 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 384 */
    nullptr,
    &sys_rseq,
    nullptr,
    nullptr,
    nullptr, /* 389 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 394 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 399 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 404 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 409 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 414 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 419 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 424 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 429 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 434 */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 439 */
};
// clang-format on

void syscall::syscallHandler(isr::registers *regs) {
    if (regs->eax >= (sizeof(syscall_table) / sizeof(syscall_table[0])) || syscall_table[regs->eax] == nullptr) {
        DEBUG_PRINTF("syscall %u not found\n", regs->eax);
        regs->eax = -ENOSYS;
        return;
    }
    DEBUG_PRINTF_INSANE("calling syscall %u\n", regs->eax);
    int syscall_ret;
    uint32_t returnval = syscall_table[regs->eax](regs, &syscall_ret, regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->ebp);
    if (syscall_ret > 0) {
        regs->eax = returnval;
        DEBUG_PRINTF_INSANE("syscall return %d\n", returnval);
    }
}
