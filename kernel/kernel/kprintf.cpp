#include <string.h>
#include <vix/config.h>
#include <vix/interrupts.h>
#include <vix/kprintf.h>
#include <vix/macros.h>
#include <vix/sched.h>
#include <vix/stdio.h>
#include <vix/time.h>
#include <vix/types.h>

#define TMP_BUF_LEN (512)

#ifdef CONFIG_KPRINTF_ENABLE_BUF
static char kp_buf[CONFIG_KPRINTF_BUFSIZE];
#endif // CONFIG_KPRINTF_ENABLE_BUF
static char kp_buf_tmp[TMP_BUF_LEN];

#define KP_INFO_MAGIC (0x4B010550)
struct __attribute__((packed)) kp_buf_info {
    uint32_t magic; // KP_INFO_MAGIC

    size_t len;
    bool last;

    int loglevel;
    uint64_t time;
};

inline void puts_kbuf(const char *s, size_t n, int loglevel) {
    if (unlikely(loglevel <= KP_ALERT)) {
        puts_sized(s, n, false);
        return; // FIXME: this is a workaround for double printing
    }
    puts_sized(s, n, true);
}

inline void puts_kbuf(const char *s, int loglevel) {
    puts_kbuf(s, strlen(s), loglevel);
}

static size_t log10(size_t n) {
    size_t r = 0;
    while (n >= 10) {
        n /= 10;
        r++;
    }
    return r;
}

static void print_kbuf(struct kp_buf_info *info, size_t idx, const char *buf) {
    if (info->magic != KP_INFO_MAGIC) {
        return;
    }
    int tid = -1;
    if (sched::mytask() != nullptr) {
        tid = sched::mytask()->tid;
    }

    // shift results in precision loss but it's fast
    size_t secs = (size_t)(info->time >> 20) / 1000;
    size_t ms = (size_t)(info->time >> 20) % 1000;
    char zeros[] = "00";
    zeros[2 - log10(ms)] = '\0';
    char w_buf[19];
    snprintf(w_buf, 19, "<%d>[%u.%s%u][%d] ", info->loglevel, secs, zeros, ms, tid);
    puts_kbuf(w_buf, info->loglevel);
    puts_kbuf(&buf[idx], info->len, info->loglevel);
    // puts_kbuf(&'\n', 1, info->loglevel);
}

#ifdef CONFIG_KPRINTF_ENABLE_BUF
static void writeout_kbuf() {
    for (size_t i = 0; i < CONFIG_KPRINTF_BUFSIZE;) {
        struct kp_buf_info *infoptr = (struct kp_buf_info *)&kp_buf[i];
        if (infoptr->magic != KP_INFO_MAGIC) {
            i++;
            continue;
        }
        i += sizeof(struct kp_buf_info);
        print_kbuf(infoptr, i, kp_buf);
        i += infoptr->len;
    }
}

static size_t kbuf_idx = 0;
struct kp_buf_info *last_written = nullptr;

static void write_kbuf(struct kp_buf_info info, char *src, size_t n) {
    if ((n + sizeof(struct kp_buf_info)) >= CONFIG_KPRINTF_BUFSIZE) {
        // there is no way we could possibly store this message
        // TODO: send it out to the terminals anyway
        return;
    }
    if ((kbuf_idx + sizeof(struct kp_buf_info) + n) < CONFIG_KPRINTF_BUFSIZE) {
        if (kbuf_idx != 0) {
            last_written->last = false;
        }
        struct kp_buf_info *infoptr = (struct kp_buf_info *)&kp_buf[kbuf_idx];
        kbuf_idx += sizeof(struct kp_buf_info);
        (*infoptr) = info;
        infoptr->magic = KP_INFO_MAGIC;
        infoptr->len = n;
        infoptr->last = true;
        memcpy(&kp_buf[kbuf_idx], src, n);
        print_kbuf(infoptr, kbuf_idx, kp_buf);
        kbuf_idx += n;
        last_written = infoptr;
    } else {
        kbuf_idx = 0;
        /*puts_kbuf("!!!buffer overflowed!!! -- BEGIN\n", 0);
        writeout_kbuf();
        puts_kbuf("!!!buffer overflowed!!! -- END\n", 0);*/
        write_kbuf(info, src, n);
    }
}
#else  // CONFIG_KPRINTF_ENABLE_BUF
static void write_kbuf(struct kp_buf_info info, char *src, size_t n) {
    info.magic = KP_INFO_MAGIC;
    info.len = n;
    print_kbuf(&info, 0, src);
}
#endif // CONFIG_KPRINTF_ENABLE_BUF

static int current_loglevel = CONFIG_KPRINTF_LOGLEVEL;

extern "C" void vkprintf(int loglevel, const char *fmt, va_list args) {
    // FIXME: lock instead of a critical section lmfao
    if (loglevel <= current_loglevel) {
        push_interrupt_disable();
        struct kp_buf_info info;
        info.time = time::ns_since_bootup;
        info.loglevel = loglevel;

        int n = vsnprintf(kp_buf_tmp, TMP_BUF_LEN, fmt, args);
        // error?
        if (n < 0) {
            n = 0;
        }
        // vsnprintf returns the number of characters that _would_ have been written if the buffer was big enough
        if (n > TMP_BUF_LEN) {
            n = TMP_BUF_LEN;
        }
        write_kbuf(info, kp_buf_tmp, n);
        pop_interrupt_disable();
    }
}

extern "C" void kprintf(int loglevel, const char *fmt, ...) {
    if (loglevel > current_loglevel) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vkprintf(loglevel, fmt, args);
    va_end(args);
}
