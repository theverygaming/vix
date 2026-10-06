#include <vix/debug.h>
#include <vix/abi/linux/errno.h>
#include <vix/kprintf.h>
#include <vix/arch/monitor.h>
#include <vix/arch/linux-syscalls.h>
#include <vix/panic.h>

// FIXME: we should unmap kernel pages when we switch to userspace :P

static unsigned long monitor_flags = 0;
static uintptr_t trap_handler = 0;
static uintptr_t kernel_stack = 0;
static linux_pid_t childpid;
#define MONITOR_CALL_SET_TRAP_HANDLER 1
#define MONITOR_CALL_SET_FLAGS 2
#define MONITOR_CALL_GET_FLAGS 3
#define MONITOR_CALL_SET_KERNEL_STACK 4
static uint8_t *kpmem;
static size_t kpmem_size;

#define CHK_ERR(expr) do { \
    int val = (expr); \
    if (val < 0) { \
        kprintf(KP_ALERT, "encountered error %d " __FILE__ ":" STRINGIFY(__LINE__) "\n", val); \
        linux_exit(1); \
    } \
} while(0)

static void signal_handler(int sig) {
    if ((monitor_flags & MONITOR_FLAG_TIMER) != 0) {
        linux_kill(childpid, LINUX_SIGSTOP);
    }
}

static void setup_timer() {
    struct linux_sigaction sigaction {
        .sa_handler = signal_handler,
        .sa_flags = LINUX_SA_RESTORER | LINUX_SA_RESTART,
        .sa_restorer = linux_signal_restorer,
        .sa_mask = 0,
    };
    CHK_ERR(linux_rt_sigaction(LINUX_SIGALRM, &sigaction, nullptr));
    struct linux_sigevent sigevent {
        .sigev_value = { .sival_ptr = nullptr },
        .sigev_signo = LINUX_SIGALRM,
        .sigev_notify = LINUX_SIGEV_SIGNAL,
        .sigev_tid = 0,
    };
    linux_timer_t timer_id;
    CHK_ERR(linux_timer_create(LINUX_CLOCK_MONOTONIC, &sigevent, &timer_id));
    struct linux_itimerspec itimerspec = {
        .it_interval = {
            .tv_sec = 0,
            .tv_nsec = (50 * 1000000), // 50ms
        },
        .it_value = {
            .tv_sec = 0,
            .tv_nsec = (50 * 1000000), // 50ms
        },
    };
    CHK_ERR(linux_timer_settime(timer_id, 0, &itimerspec, nullptr));
}

static void dumpchildregs(struct linux_user_regs_struct *regs) {
    kprintf(KP_ALERT, "dumpchildregs: r15 0x%p\n", regs->r15);
    kprintf(KP_ALERT, "dumpchildregs: r14 0x%p\n", regs->r14);
    kprintf(KP_ALERT, "dumpchildregs: r13 0x%p\n", regs->r13);
    kprintf(KP_ALERT, "dumpchildregs: r12 0x%p\n", regs->r12);
    kprintf(KP_ALERT, "dumpchildregs: bp 0x%p\n", regs->bp);
    kprintf(KP_ALERT, "dumpchildregs: bx 0x%p\n", regs->bx);
    kprintf(KP_ALERT, "dumpchildregs: r11 0x%p\n", regs->r11);
    kprintf(KP_ALERT, "dumpchildregs: r10 0x%p\n", regs->r10);
    kprintf(KP_ALERT, "dumpchildregs: r9 0x%p\n", regs->r9);
    kprintf(KP_ALERT, "dumpchildregs: r8 0x%p\n", regs->r8);
    kprintf(KP_ALERT, "dumpchildregs: ax 0x%p\n", regs->ax);
    kprintf(KP_ALERT, "dumpchildregs: cx 0x%p\n", regs->cx);
    kprintf(KP_ALERT, "dumpchildregs: dx 0x%p\n", regs->dx);
    kprintf(KP_ALERT, "dumpchildregs: si 0x%p\n", regs->si);
    kprintf(KP_ALERT, "dumpchildregs: di 0x%p\n", regs->di);
    kprintf(KP_ALERT, "dumpchildregs: orig_ax 0x%p\n", regs->orig_ax);
    kprintf(KP_ALERT, "dumpchildregs: ip 0x%p\n", regs->ip);
    kprintf(KP_ALERT, "dumpchildregs: cs 0x%p\n", regs->cs);
    kprintf(KP_ALERT, "dumpchildregs: flags 0x%p\n", regs->flags);
    kprintf(KP_ALERT, "dumpchildregs: sp 0x%p\n", regs->sp);
    kprintf(KP_ALERT, "dumpchildregs: ss 0x%p\n", regs->ss);
    kprintf(KP_ALERT, "dumpchildregs: fs_base 0x%p\n", regs->fs_base);
    kprintf(KP_ALERT, "dumpchildregs: gs_base 0x%p\n", regs->gs_base);
    kprintf(KP_ALERT, "dumpchildregs: ds 0x%p\n", regs->ds);
    kprintf(KP_ALERT, "dumpchildregs: es 0x%p\n", regs->es);
    kprintf(KP_ALERT, "dumpchildregs: fs 0x%p\n", regs->fs);
    kprintf(KP_ALERT, "dumpchildregs: gs 0x%p\n", regs->gs);
}

static void trapchild(struct linux_user_regs_struct *regs, uint64_t icode, uint64_t imeta1, uint64_t imeta2, uint64_t imeta3, uint64_t imeta4) {
    uint64_t orig_sp = regs->sp;
    unsigned long orig_monitor_flags = monitor_flags;

    if ((monitor_flags & MONITOR_FLAG_USERMODE) != 0) {
        regs->sp = kernel_stack;
        monitor_flags &= ~(MONITOR_FLAG_USERMODE); // switch to kernel mode
    }

    regs->sp = ALIGN_DOWN(regs->sp, 16); // the stack shall be 16-byte aligned as x86_64 commands!
    // push rsp
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)orig_sp));

    // push rip
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)regs->ip));

    // push cs
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)regs->cs));

    // push rflags
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)regs->flags));

    // push rcx (clobbered by syscall instruction)
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)regs->cx));

    // push r11 (clobbered by syscall instruction)
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)regs->r11));

    // push rax (clobbered by syscall args)
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)regs->ax));

    // push rdi (clobbered by syscall args)
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)regs->di));

    // push monitor flags
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)orig_monitor_flags));

    // starting from here these won't be restored by MONITOR_CALL_TRAPRET

    // push interrupt code
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)icode));

    // push interrupt metadata 1
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)imeta1));

    // push interrupt metadata 2
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)imeta2));

    // push interrupt metadata 3
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)imeta3));

    // push interrupt metadata 4
    regs->sp -= 8;
    CHK_ERR(linux_ptrace(LINUX_PTRACE_POKEDATA, childpid, (void *)regs->sp, (void *)imeta4));

    regs->ip = trap_handler;

    // unset the timer flag, to avoid nested interrupts - exceptions are an.. exception :)
    monitor_flags &= ~(MONITOR_FLAG_TIMER);
}

static void dumpchildregs() {
    struct linux_user_regs_struct regs;
    struct linux_iovec iov = {
        .iov_base = &regs,
        .iov_len = sizeof(regs)
    };
    CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
    dumpchildregs(&regs);
}

void monitor_entry(linux_pid_t _childpid, int memfd, size_t memfd_bytes) {
    childpid = _childpid;
    kpmem = (uint8_t *)linux_mmap(nullptr, kpmem_size, LINUX_PROT_READ | LINUX_PROT_WRITE, LINUX_MAP_SHARED, memfd, 0);
    kpmem_size = memfd_bytes;
    kprintf(KP_INFO, "monitor process started (PID %d)\n", linux_getpid());

    int status;
    CHK_ERR(linux_wait4(childpid, &status, 0, nullptr));
    setup_timer();
    CHK_ERR(linux_ptrace(LINUX_PTRACE_SETOPTIONS, childpid, 0, (void *)LINUX_PTRACE_O_TRACESYSGOOD));
    while(true) {
        CHK_ERR(linux_ptrace((monitor_flags & MONITOR_FLAG_USERMODE) != 0 ? LINUX_PTRACE_SYSEMU : LINUX_PTRACE_SYSCALL, childpid, nullptr, nullptr));
        CHK_ERR(linux_wait4(childpid, &status, 0, nullptr));

        if (LINUX_WIFSTOPPED(status)) {
            int sig = LINUX_WSTOPSIG(status);
            // segfault
            if (sig == LINUX_SIGSEGV) {
                struct linux_user_regs_struct regs;
                struct linux_iovec iov = {
                    .iov_base = &regs,
                    .iov_len = sizeof(regs)
                };
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                linux_siginfo_t siginfo;
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETSIGINFO, childpid, nullptr, &siginfo));
                trapchild(&regs, MONITOR_TRAPCODE_SEGV, (uint64_t)siginfo.si_addr, 0, 0, 0);
                DEBUG_PRINTF("monitor: child SIGSEGV IP: 0x%p fault addr: 0x%p\n", regs.ip, siginfo.si_addr);
                CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
            } else if (sig == LINUX_SIGFPE) { // div by zero n shit
                struct linux_user_regs_struct regs;
                struct linux_iovec iov = {
                    .iov_base = &regs,
                    .iov_len = sizeof(regs)
                };
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                trapchild(&regs, MONITOR_TRAPCODE_ME, 0, 0, 0, 0);
                DEBUG_PRINTF("monitor: child SIGFPE IP: 0x%p\n", regs.ip);
                CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
            } else if (sig == (LINUX_SIGTRAP | 0x80)) {
                struct linux_user_regs_struct regs;
                struct linux_iovec iov = {
                    .iov_base = &regs,
                    .iov_len = sizeof(regs)
                };
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));

                if ((monitor_flags & MONITOR_FLAG_USERMODE) != 0) {
                    regs.ax = regs.orig_ax;
                    trapchild(&regs, MONITOR_TRAPCODE_SYSCALL, 0, 0, 0, 0);
                    CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                    // we must now catch the system call exit, usually LINUX_PTRACE_SYSEMU expects
                    // one would call ptrace(LINUX_PTRACE_SYSEMU again, not ptrace(LINUX_PTRACE_SYSCALL
                    // if we didn't do this LINUX_PTRACE_SYSCALL entry & exit would get out of sync and absolutely everything would explode!
                    CHK_ERR(linux_ptrace(LINUX_PTRACE_SYSCALL, childpid, nullptr, nullptr));
                    CHK_ERR(linux_wait4(childpid, &status, 0, nullptr));
                    if (!(LINUX_WIFSTOPPED(status) && LINUX_WSTOPSIG(status) == (LINUX_SIGTRAP | 0x80))) {
                        kprintf(KP_ALERT, "WTF? " __FILE__ ":" STRINGIFY(__LINE__) " status: 0x%p\n", status);
                        break;
                    }
                    continue;
                }

                bool syscall_emulate = false;

                // catch and intercept monitor calls (magic syscall number)
                if (regs.orig_ax == MONITOR_CALL) {
                    syscall_emulate = true;
                    switch(regs.di) {
                        case MONITOR_CALL_TRAPRET: {
                            // pop monitor flags
                            unsigned long flags;
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &flags));
                            regs.sp += 8;
                            monitor_flags = (flags & (MONITOR_FLAG_USERMODE | MONITOR_FLAG_TIMER));

                            // pop rdi
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.di));
                            regs.sp += 8;

                            // pop rax
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.ax));
                            regs.sp += 8;

                            // pop r11
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.r11));
                            regs.sp += 8;

                            // pop rcx
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.cx));
                            regs.sp += 8;

                            // pop rflags
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.flags));
                            regs.sp += 8;

                            // pop cs
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.cs));
                            regs.sp += 8;

                            // pop rip
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.ip));
                            regs.sp += 8;

                            // pop rsp
                            CHK_ERR(linux_ptrace(LINUX_PTRACE_PEEKDATA, childpid, (void *)regs.sp, &regs.sp));
                            // no need to change rsp after this :)

                            break;
                        }
                        case MONITOR_CALL_SET_FLAGS: {
                            unsigned long untouchable_flags = MONITOR_FLAG_USERMODE;
                            unsigned long newflags = regs.si;
                            newflags &= ~untouchable_flags;
                            newflags |= monitor_flags & untouchable_flags;
                            regs.ax = monitor_flags;
                            monitor_flags = newflags;
                            break;
                        }
                        case MONITOR_CALL_SET_TRAP_HANDLER: {
                            trap_handler = regs.si;
                            break;
                        }
                        case MONITOR_CALL_GET_FLAGS: {
                            regs.ax = monitor_flags;
                            break;
                        }
                        case MONITOR_CALL_SET_KERNEL_STACK: {
                            kernel_stack = regs.si;
                            break;
                        }
                        default: {
                            regs.ax = -1;
                            break;
                        }
                    }
                }

                if (syscall_emulate) {
                    regs.orig_ax = -1; // execute a bogus syscall (will return ENOSYS), we will overwrite the return value later
                    CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                }

                // continue and catch syscall exit
                CHK_ERR(linux_ptrace(LINUX_PTRACE_SYSCALL, childpid, nullptr, nullptr));
                CHK_ERR(linux_wait4(childpid, &status, 0, nullptr));
                if (!(LINUX_WIFSTOPPED(status) && LINUX_WSTOPSIG(status) == (LINUX_SIGTRAP | 0x80))) {
                    if (LINUX_WIFEXITED(status)) {
                        // normal exit
                        break;
                    }
                    kprintf(KP_ALERT, "WTF? " __FILE__ ":" STRINGIFY(__LINE__) " status: 0x%p\n", status);
                    break;
                }

                if (syscall_emulate) {
                    CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                } else {
                    CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                    // detect the kernel trying to restart a syscall, and prevent it from doing so. We restart syscalls ourselves in this household!!!
                    // this must be done because otherwise we'd explode the linux syscall restart process
                    // by jumping somewhere else when we restore registers in the timer interrupt
                    if ((int64_t)regs.ax == -LINUX_ERESTARTSYS || (int64_t)regs.ax == -LINUX_ERESTARTNOINTR) {
                        // gaslight the linux kernel so it doesn't explode everything
                        // https://github.com/torvalds/linux/blob/a74306e2e676f9775457366fc047a660fbf02f26/arch/x86/kernel/signal.c#L266-L282
                        regs.ax = regs.orig_ax;
                        regs.ip -= 2; // go back to syscall instruction
                        DEBUG_PRINTF("detected syscall restart attmept, manually restarting instead!\n");
                        CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                    }
                }
            } else if (sig == LINUX_SIGSTOP) {
                struct linux_user_regs_struct regs;
                struct linux_iovec iov = {
                    .iov_base = &regs,
                    .iov_len = sizeof(regs)
                };
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));

                trapchild(&regs, MONITOR_TRAPCODE_TIMER, 0, 0, 0, 0);

                CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
            } else {
                kprintf(KP_WARNING, "unknown stopped signal %d (status: 0x%p)\n", sig, status);
                dumpchildregs();
                //break;
            }
        } else if (LINUX_WIFEXITED(status)) {
            // normal exit
            break;
        } else {
            kprintf(KP_ALERT, "WTF? " __FILE__ ":" STRINGIFY(__LINE__) " status: 0x%p\n", status);
            dumpchildregs();
            break;
        }
    }
}

void monitor_set_trap_handler(uintptr_t addr) {
    linux_syscall2(MONITOR_CALL, MONITOR_CALL_SET_TRAP_HANDLER, addr);
}

unsigned long monitor_set_flags(unsigned long flags) {
    return linux_syscall2(MONITOR_CALL, MONITOR_CALL_SET_FLAGS, flags);
}

unsigned long monitor_get_flags() {
    return linux_syscall1(MONITOR_CALL, MONITOR_CALL_GET_FLAGS);
}

void monitor_set_kernel_stack(uintptr_t addr) {
    linux_syscall2(MONITOR_CALL, MONITOR_CALL_SET_KERNEL_STACK, addr);
}
