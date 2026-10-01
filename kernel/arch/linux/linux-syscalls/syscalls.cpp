#include <vix/arch/linux-syscalls.h>
#include <stdarg.h>

ssize_t linux_read(unsigned int fd, void *buf, size_t count) {
    return linux_syscall3(0, fd, (uint64_t)buf, count);
}

ssize_t linux_write(unsigned int fd, const void *buf, size_t count) {
    return linux_syscall3(1, fd, (uint64_t)buf, count);
}

int linux_open(const char *filename, int flags, linux_umode_t mode) {
    return linux_syscall3(2, (uint64_t)filename, flags, mode);
}

int linux_poll(struct linux_pollfd *fds, unsigned int nfds, int timeout) {
    return linux_syscall3(7, (uint64_t)fds, nfds, timeout);
}

void *linux_mmap(void *addr, size_t length, int prot, int flags, int fd, ssize_t offset) {
    return (void *)linux_syscall6(9, (uint64_t)addr, length, prot, flags, fd, offset);
}

int linux_mprotect(void *start, size_t length, int prot) {
    return linux_syscall3(10, (uint64_t)start, length, prot);
}

int linux_munmap(void *addr, size_t length) {
    return linux_syscall2(11, (uint64_t)addr, length);
}

int linux_rt_sigaction(int sig, const struct linux_sigaction *act, struct linux_sigaction *oact) {
    return linux_syscall4(13, sig, (uint64_t)act, (uint64_t)oact, sizeof(linux_sigset_t));
}

int linux_rt_sigprocmask(int how, linux_sigset_t *newset, linux_sigset_t *oldset) {
    return linux_syscall4(14, how, (uint64_t)newset, (uint64_t)oldset, sizeof(linux_sigset_t));
}

int linux_ioctl(int fd, unsigned int cmd, unsigned long arg) {
    return linux_syscall3(16, fd, cmd, (uint64_t)arg);
}

linux_pid_t linux_getpid() {
    return linux_syscall0(39);
}

linux_pid_t linux_fork() {
    return linux_syscall0(57);
}

int linux_exit(int error_code) {
    return linux_syscall1(60, error_code);
}

linux_pid_t linux_wait4(linux_pid_t upid, int *stat_addr, int options, void *ru) {
    return linux_syscall4(61, upid, (uint64_t)stat_addr, options, (uint64_t)ru);
}

int linux_kill(linux_pid_t pid, int sig) {
    return linux_syscall2(62, pid, sig);
}

int linux_ftruncate(int fd, unsigned long length) {
    return linux_syscall2(77, fd, length);
}

ssize_t linux_ptrace(int request, linux_pid_t pid, void *addr, void *data) {
    return linux_syscall4(101, request, pid, (uint64_t)addr, (uint64_t)data);
}

int linux_prctl(int op, ...) {
    uint64_t arg2 = 0;
    uint64_t arg3 = 0;
    uint64_t arg4 = 0;
    uint64_t arg5 = 0;
    uint64_t arg6 = 0;

    va_list args;
    va_start(args, op);
    switch (op) {
        case LINUX_PR_SET_PDEATHSIG:
            arg2 = va_arg(args, long);
            break;
        default:
            va_end(args);
            return -LINUX_ENOSYS;
    }
    va_end(args);

    return linux_syscall6(157, op, arg2, arg3, arg4, arg5, arg6);
}

linux_pid_t linux_gettid() {
    return linux_syscall0(186);
}

int linux_timer_create(const linux_clockid_t which_clock, struct linux_sigevent *timer_event_spec, linux_timer_t *created_timer_id) {
    return linux_syscall3(222, which_clock, (uint64_t)timer_event_spec, (uint64_t)created_timer_id);
}

int linux_timer_settime(linux_timer_t timer_id, int flags, const struct linux_itimerspec *new_setting, struct linux_itimerspec *old_setting) {
    return linux_syscall4(223, timer_id, flags, (uint64_t)new_setting, (uint64_t)old_setting);
}

int linux_tgkill(linux_pid_t tgid, linux_pid_t pid, int sig) {
    return linux_syscall3(234, tgid, pid, sig);
}

int linux_timerfd_create(int clockid, int flags) {
    return linux_syscall2(283, clockid, flags);
}

int linux_timerfd_settime(int ufd, int flags, const struct linux_itimerspec *utmr, struct linux_itimerspec *otmr) {
    return linux_syscall4(286, ufd, flags, (uint64_t)utmr, (uint64_t)otmr);
}

int linux_signalfd4(int ufd, linux_sigset_t *user_mask, int flags) {
    return linux_syscall4(289, ufd, (uint64_t)user_mask, sizeof(linux_sigset_t), flags);
}

int linux_memfd_create(const char *uname, unsigned int flags) {
    return linux_syscall2(319, (uint64_t)uname, flags);
}

int linux_pidfd_open(linux_pid_t pid, unsigned int flags) {
    return linux_syscall2(434, pid, flags);
}
