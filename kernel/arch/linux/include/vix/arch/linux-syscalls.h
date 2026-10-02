#pragma once
#include <vix/types.h>
#include <vix/arch/linux-syscalls.h>
#include <vix/abi/linux/errno.h>

extern "C" uint64_t linux_syscall(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
extern "C" uint64_t linux_syscall0(uint64_t) __asm__("linux_syscall");
extern "C" uint64_t linux_syscall1(uint64_t, uint64_t) __asm__("linux_syscall");
extern "C" uint64_t linux_syscall2(uint64_t, uint64_t, uint64_t) __asm__("linux_syscall");
extern "C" uint64_t linux_syscall3(uint64_t, uint64_t, uint64_t, uint64_t) __asm__("linux_syscall");
extern "C" uint64_t linux_syscall4(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) __asm__("linux_syscall");
extern "C" uint64_t linux_syscall5(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) __asm__("linux_syscall");
extern "C" uint64_t linux_syscall6(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) __asm__("linux_syscall");

typedef int linux_pid_t;

struct linux_user_regs_struct {
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long bp;
    unsigned long bx;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long ax;
    unsigned long cx;
    unsigned long dx;
    unsigned long si;
    unsigned long di;
    unsigned long orig_ax;
    unsigned long ip;
    unsigned long cs;
    unsigned long flags;
    unsigned long sp;
    unsigned long ss;
    unsigned long fs_base;
    unsigned long gs_base;
    unsigned long ds;
    unsigned long es;
    unsigned long fs;
    unsigned long gs;
};

struct linux_iovec {
    void   *iov_base;
    size_t  iov_len;
};

typedef long long linux_time_t;

struct linux_timespec {
    linux_time_t tv_sec;
    long tv_nsec;
};

struct linux_itimerspec {
    struct linux_timespec it_interval;
    struct linux_timespec it_value;
};

struct linux_pollfd {
    int fd;
    short events;
    short revents;
};

typedef unsigned long linux_sigset_t;

struct linux_sigaction {
    void (*sa_handler)(int);
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    linux_sigset_t sa_mask;
};

typedef int linux_clockid_t;

typedef int linux_timer_t;

union linux_sigval {
    int sival_int;
    void *sival_ptr;
};

struct linux_sigevent {
    union linux_sigval sigev_value;
    int sigev_signo;
    int sigev_notify;
    int sigev_tid;
};

typedef unsigned short linux_umode_t;

// docs: https://github.com/torvalds/linux/blob/551c722f40809618230001baccf219193e22fc5a/include/uapi/linux/fs.h#L526-L558
enum linux_procmap_query_flags {
    LINUX_PROCMAP_QUERY_VMA_READABLE = 0x01,
    LINUX_PROCMAP_QUERY_VMA_WRITABLE = 0x02,
    LINUX_PROCMAP_QUERY_VMA_EXECUTABLE = 0x04,
    LINUX_PROCMAP_QUERY_VMA_SHARED = 0x08,
    LINUX_PROCMAP_QUERY_COVERING_OR_NEXT_VMA = 0x10,
    LINUX_PROCMAP_QUERY_FILE_BACKED_VMA = 0x20,
};

// docs: https://github.com/torvalds/linux/blob/551c722f40809618230001baccf219193e22fc5a/include/uapi/linux/fs.h#L583-L675
struct linux_procmap_query {
    uint64_t size; // sizeof(struct linux_procmap_query)
    uint64_t query_flags;
    uint64_t query_addr;
    uint64_t vma_start;
    uint64_t vma_end;
    uint64_t vma_flags;
    uint64_t vma_page_size;
    uint64_t vma_offset;
    uint64_t inode;
    uint32_t dev_major;
    uint32_t dev_minor;
    uint32_t vma_name_size;
    uint32_t build_id_size;
    uint64_t vma_name_addr;
    uint64_t build_id_addr;
};

typedef struct {
    union {
        struct {
            int si_signo;
            int si_code;
            int si_errno;
            union {
                struct {
                    void *si_addr;
                };
            };
        };
        int _si_pad[128/sizeof(int)];
    };
} linux_siginfo_t;

#define LINUX_NSIG 32

#define LINUX_CLOCK_REALTIME 0
#define LINUX_CLOCK_MONOTONIC 1

/* NOTE: signal numbers are arch-specific. These are x86_64 */
#define LINUX_SIGTRAP 5
#define LINUX_SIGFPE 8
#define LINUX_SIGKILL 9
#define LINUX_SIGSEGV 11
#define LINUX_SIGALRM 14
#define LINUX_SIGCHLD 17
#define LINUX_SIGSTOP 19

#define LINUX_SIG_BLOCK 0

#define LINUX_SA_RESTORER 0x04000000
#define LINUX_SA_RESTART 0x10000000

#define LINUX_O_NONBLOCK (1 << 11)
#define LINUX_O_CLOEXEC (1 << 19)

#define LINUX_SIGEV_SIGNAL 0
#define LINUX_SIGEV_NONE 1
#define LINUX_SIGEV_THREAD 2
#define LINUX_SIGEV_THREAD_ID 4

#define LINUX_SFD_NONBLOCK LINUX_O_NONBLOCK
#define LINUX_SFD_CLOEXEC LINUX_O_CLOEXEC

#define LINUX_PR_SET_PDEATHSIG 1

#define LINUX_PTRACE_TRACEME 0
#define LINUX_PTRACE_CONT 7
#define LINUX_PTRACE_SYSCALL 24
#define LINUX_PTRACE_SYSEMU 31 // I think this is x86_64-specific?
#define LINUX_PTRACE_GETSIGINFO 0x4202
#define LINUX_PTRACE_GETREGSET 0x4204
#define LINUX_PTRACE_SETREGSET 0x4205
#define LINUX_PTRACE_INTERRUPT 0x4207
#define LINUX_PTRACE_SETOPTIONS 0x4200
#define LINUX_PTRACE_O_TRACESYSGOOD 1

#define LINUX_WIFEXITED(status) (((status) & 0x7f) == 0)
#define LINUX_WIFSTOPPED(status) (((status) & 0xff) == 0x7f)
#define LINUX_WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#define LINUX_WSTOPSIG(status) (LINUX_WEXITSTATUS(status))

#define LINUX_NT_PRSTATUS 1

#define LINUX_AT_SYSINFO_EHDR 33 // x86_64-specific I think

#define LINUX_PROT_NONE 0x0
#define LINUX_PROT_READ 0x1
#define LINUX_PROT_WRITE 0x2
#define LINUX_PROT_EXEC 0x4
#define LINUX_PROT_SEM 0x8

#define LINUX_MAP_SHARED 0x01
#define LINUX_MAP_PRIVATE 0x02
#define LINUX_MAP_FIXED 0x10

#define LINUX_POLLIN 0x0001

#define LINUX_WNOHANG 0x00000001

#define LINUX_O_RDONLY 0

#define LINUX_PROCMAP_QUERY 0xc0686611

extern "C" void linux_signal_restorer(void);

ssize_t linux_read(unsigned int fd, void *buf, size_t count);
ssize_t linux_write(unsigned int fd, const void *buf, size_t count);
int linux_open(const char *filename, int flags, linux_umode_t mode);
int linux_close(int fd);
int linux_poll(struct linux_pollfd *fds, unsigned int nfds, int timeout);
void *linux_mmap(void *addr, size_t length, int prot, int flags, int fd, ssize_t offset);
int linux_mprotect(void *start, size_t length, int prot);
int linux_munmap(void *addr, size_t length);
int linux_rt_sigaction(int sig, const struct linux_sigaction *act, struct linux_sigaction *oact);
int linux_rt_sigprocmask(int how, linux_sigset_t *newset, linux_sigset_t *oldset);
int linux_ioctl(int fd, unsigned int cmd, unsigned long arg);
linux_pid_t linux_getpid();
linux_pid_t linux_fork();
int linux_exit(int error_code);
linux_pid_t linux_wait4(linux_pid_t upid, int *stat_addr, int options, void *ru);
int linux_kill(linux_pid_t pid, int sig);
int linux_ftruncate(int fd, unsigned long length);
ssize_t linux_ptrace(int request, linux_pid_t pid, void *addr, void *data);
int linux_prctl(int op, ...);
linux_pid_t linux_gettid();
int linux_timer_create(const linux_clockid_t which_clock, struct linux_sigevent *timer_event_spec, linux_timer_t *created_timer_id);
int linux_timer_settime(linux_timer_t timer_id, int flags, const struct linux_itimerspec *new_setting, struct linux_itimerspec *old_setting);
int linux_tgkill(linux_pid_t tgid, linux_pid_t pid, int sig);
int linux_timerfd_create(int clockid, int flags);
int linux_timerfd_settime(int ufd, int flags, const struct linux_itimerspec *utmr, struct linux_itimerspec *otmr);
int linux_signalfd4(int ufd, linux_sigset_t *user_mask, int flags);
int linux_memfd_create(const char *uname, unsigned int flags);
int linux_pidfd_open(linux_pid_t pid, unsigned int flags);
