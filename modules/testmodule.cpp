#include <arch/generic/textoutput.h>
#include <kprintf.h>
#include <mm/kmalloc.h>
#include <module.h>
#include <stdlib.h>
#include <time.h>
#include <types.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("funny");
MODULE_VERSION("0.621");

static char *ptr;

static int init() {
    kprintf(KP_INFO, "testmod: loaded\n");
    kprintf(KP_INFO, "testmod: hello world from module! %d\n", 5);
    kprintf(KP_INFO, "testmod: current unix time: %u\n", time::getCurrentUnixTime());
    ptr = (char *)mm::kmalloc(100);
    memcpy(ptr, "hello world!\n", 14);
    return 0;
}

static void exit() {
    kprintf(KP_INFO, "testmod: %s", ptr);
    mm::kfree(ptr);
    kprintf(KP_INFO, "testmod: module gon\n");
}

MODULE_INIT(init);
MODULE_EXIT(exit);
