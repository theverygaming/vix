#include <vix/abi/linux/errno.h>
#include <vix/kprintf.h>
#include <vix/arch/monitor.h>
#include <vix/arch/linux-syscalls.h>
#include <vix/panic.h>

static bool usermode = false;
static linux_pid_t childpid;
static volatile bool run_timer = true;
#define SYS_MONITOR_CALL 0xABCD
static uint8_t *kpmem;
static size_t kpmem_size;

#define CHK_ERR(expr) do { \
    int val = (expr); \
    if (val < 0) { \
        kprintf(KP_ALERT, "ecountered error %d\n", val); \
        linux_exit(1); \
    } \
} while(0)

static void signal_handler(int sig) {
    if (run_timer) {
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
    while(true) {
        CHK_ERR(linux_ptrace(usermode ? LINUX_PTRACE_SYSEMU : LINUX_PTRACE_SYSCALL, childpid, nullptr, nullptr));
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
                kprintf(KP_ALERT, "monitor: child SIGSEGV IP: 0x%p fault addr: 0x%p\n", regs.ip, siginfo.si_addr);
                dumpchildregs();
                break;
            } else if (sig == LINUX_SIGFPE) { // div by zero n shit
                struct linux_user_regs_struct regs;
                struct linux_iovec iov = {
                    .iov_base = &regs,
                    .iov_len = sizeof(regs)
                };
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                kprintf(KP_ALERT, "monitor: child SIGFPE IP: 0x%p\n", regs.ip);
                dumpchildregs();
                break;
            } else if (sig == LINUX_SIGTRAP) {
                struct linux_user_regs_struct regs;
                struct linux_iovec iov = {
                    .iov_base = &regs,
                    .iov_len = sizeof(regs)
                };
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));

                // kprintf(KP_INFO, "monitor got syscall %u\n", regs.orig_ax);

                if (!usermode) {
                    // catch and intercept monitor calls (magic syscall number)
                    bool syscall_emulate = false;
                    unsigned long syscall_emulate_ret = 0;

                    if (regs.orig_ax == SYS_MONITOR_CALL) {
                        kprintf(KP_INFO, "monitor: got monitor call\n");
                        syscall_emulate = true;
                        syscall_emulate_ret = 69;
                    }

                    if (syscall_emulate) {
                        regs.orig_ax = -1; // execute a bogus syscall (will return ENOSYS), we will overwrite the return value later
                        CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                    }

                    // continue and catch syscall exit
                    CHK_ERR(linux_ptrace(LINUX_PTRACE_SYSCALL, childpid, nullptr, nullptr));
                    CHK_ERR(linux_wait4(childpid, &status, 0, nullptr));
                    if (!(LINUX_WIFSTOPPED(status) && LINUX_WSTOPSIG(status) == LINUX_SIGTRAP)) {
                        if (LINUX_WIFEXITED(status)) {
                            break;
                        }
                        kprintf(KP_ALERT, "WTF?\n");
                        break;
                    }

                    if (syscall_emulate) {
                        regs.ax = syscall_emulate_ret;
                        CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                    }
                } else {
                    kprintf(KP_ALERT, "UNIMPLEMENTED: usermode syscall %u\n", regs.orig_ax);
                    break;
                }
            } else if (sig == LINUX_SIGSTOP) {
                kprintf(KP_INFO, "timer interrupt?\n");
                struct linux_user_regs_struct regs;
                struct linux_iovec iov = {
                    .iov_base = &regs,
                    .iov_len = sizeof(regs)
                };
                CHK_ERR(linux_ptrace(LINUX_PTRACE_GETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                //regs.ip = (unsigned long)kernel_panic;
                CHK_ERR(linux_ptrace(LINUX_PTRACE_SETREGSET, childpid, (void *)LINUX_NT_PRSTATUS, &iov));
                run_timer = false;
            } else {
                kprintf(KP_ALERT, "WTF?\n");
                dumpchildregs();
                break;
            }
        } else if (LINUX_WIFEXITED(status)) {
            // normal exit
            break;
        } else {
            kprintf(KP_ALERT, "WTF?\n");
            dumpchildregs();
            break;
        }
    }
}

int monitor_call() {
    return linux_syscall0(SYS_MONITOR_CALL);
}
