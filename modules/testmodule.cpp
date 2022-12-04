#include <module.h>
#include <stdio.h>
#include <types.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("funny");
MODULE_VERSION("0.621");

static int init() {
    printf("hello world from module! %d\n", 5);
    return 0;
}

static void exit() {
    printf("module gon\n");
}

MODULE_INIT(init);
MODULE_EXIT(exit);
