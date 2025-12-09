#include <algorithm>
#include <string.h>
#include <vix/abi/abi.h>
#include <vix/debug.h>
#include <vix/drivers/keyboard.h>
#include <vix/initfn.h>
#include <vix/kprintf.h>
#include <vix/sched.h>

#define KSH_PRINTF(...) kprintf(KP_ALERT, "ksh: " __VA_ARGS__)

static const int KSH_BUF_SIZE = 10;
static const int KSH_ARGC_MAX = 1;

static char ksh_buf[KSH_BUF_SIZE + 1];
static int ksh_buf_p = 0;
static char *ksh_arg_arr[KSH_ARGC_MAX];

static void ksh_exec(int argc, char **argv) {
    if (argc == 0) {
        return;
    }
    DEBUG_PRINTF("ksh: got command with argc %d\n", argc);
    for (int i = 0; i < argc; i++) {
        DEBUG_PRINTF("ksh: argv[%d] = '%s'\n", i, argv[i]);
    }

    if (strcmp(argv[0], "help") == 0) {
        KSH_PRINTF("-- list of commands --\n");
        KSH_PRINTF("help - displays this help menu\n");
        KSH_PRINTF("tasks - displays a compact list of tasks and their respective states\n");
        return;
    }
    if (strcmp(argv[0], "tasks") == 0) {
        KSH_PRINTF("-- list of tasks --\n");
        for (auto it = sched::sched_readyqueue.begin(); it != sched::sched_readyqueue.end(); it++) {
            KSH_PRINTF("PID: %d state: %c ABI: %s\n",
                       it->pid,
                       (it->state == sched::task::state::RUNNING) ? 'R' : 'S',
                       (it->abi_type == abi::type::KERNEL_ONLY ? "Kernel" : ((it->abi_type == abi::type::LINUX) ? "Linux" : "vix")));
        }
        return;
    }
}

static void ksh_input(char c) {
    if (c == '\b') {
        if (ksh_buf_p > 0) {
            ksh_buf_p--;
            ksh_buf[ksh_buf_p] = '\0';
        }
        return;
    }
    if (c == '\n') {
        if (ksh_buf_p != 0) {
            ksh_buf[ksh_buf_p] = '\0';
        }
        ksh_buf_p = 0;

        // split & count args
        size_t len = strlen(ksh_buf);
        if (len == 0) {
            return;
        }
        int argc = 0;
        char *str = ksh_buf;
        while (len--) {
            if (*str == ' ') {
                *str = '\0';
                argc++;
            }
            str++;
        }
        argc++; // we'd be off by one otherwise

        argc = std::min(argc, KSH_ARGC_MAX);

        // populate argv array
        str = ksh_buf;
        for (int i = 0; i < argc; i++) {
            ksh_arg_arr[i] = str;
            str += strlen(str) + 1;
        }
        ksh_exec(argc, ksh_arg_arr);
        return;
    }
    ksh_buf[ksh_buf_p] = c;
    ksh_buf_p++;
    if (ksh_buf_p >= KSH_BUF_SIZE) {
        ksh_buf_p = 0;
    }
}

static void init_ksh() {
    ksh_buf[KSH_BUF_SIZE] = '\0';
    kprintf(KP_INFO, "ksh: init\n");
    drivers::keyboard::events.register_listener(
        [](void *ctx, const char &c) {
            ksh_input(c);
            return false;
        },
        nullptr);
}

INITFN_DEFINE(ksh, INITFN_DRIVER_INIT, 0, init_ksh);
