#include <arch/x86/syscall.h>
#include <config.h>
#include <debug.h>
#include <arch/x86/multitasking.h>
#include <arch/x86/syscalls.h>

/* syscall arguments correspond to registers eax, ebx, ecx, edx, esi, edi and ebp */

uint32_t (*syscall_table[385])(isr::registers *, int *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = {
    nullptr,
    &sys_exit,
    &sys_fork,
    &sys_read,
    &sys_write, /* 4   */
    nullptr,
    nullptr,
    &sys_waitpid,
    nullptr,
    nullptr, /* 9  */
    nullptr,
    &sys_execve,
    nullptr,
    nullptr,
    nullptr, /* 14  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 19  */
    nullptr,
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
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 49  */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 54  */
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
    nullptr,
    nullptr,
    nullptr,
    nullptr, /* 119 */
    nullptr,
    nullptr,
    &sys_uname,
    &modify_ldt,
    nullptr, /* 124 */
    nullptr,
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
    nullptr,
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
    nullptr,
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
    nullptr,
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
    nullptr,
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
    nullptr,
    nullptr,
    nullptr,
    &set_thread_area,
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
    nullptr,
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
    nullptr,
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
};

void syscall::syscallHandler(isr::registers *regs) {

    if (syscall_table[regs->eax] == nullptr) {
        DEBUG_PRINTF("syscall %u not found\n", regs->eax);
        regs->eax = -1; // TODO: return correct error code
        return;
    }
    // DEBUG_PRINTF("calling syscall %u\n", regs->eax);
    int syscall_ret;
    uint32_t returnval = syscall_table[regs->eax](regs, &syscall_ret, regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->ebp);
    if(syscall_ret > 0) {
        regs->eax = returnval;
    }
}
