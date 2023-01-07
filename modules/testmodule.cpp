#include <arch/generic/textoutput.h>
#include <mm/memalloc.h>
#include <module.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <types.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("funny");
MODULE_VERSION("0.621");

static char *ptr;

static int init() {
    arch::generic::textoutput::puts("module loaded!\n", arch::generic::textoutput::color::COLOR_LIGHT_RED);
    printf("hello world from module! %d\n", 5);
    printf("current unix time: %llu\n", time::getCurrentUnixTime());
    ptr = (char *)mm::kmalloc(100);
    memcpy(ptr, "hello world!\n", 14);
    return 0;
}

static void exit() {
    printf("%s", ptr);
    mm::kfree(ptr);
    printf("module gon\n");
    arch::generic::textoutput::puts("module unloaded!\n", arch::generic::textoutput::color::COLOR_LIGHT_RED);
}

MODULE_INIT(init);
MODULE_EXIT(exit);
