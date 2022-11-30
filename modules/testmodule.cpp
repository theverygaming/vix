#include <module.h>
#include <stdio.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("funny");
MODULE_VERSION("0.621");

static int init(void) {
    printf("hello world!\n");
    return 0;
}

static void exit(void) {
    printf("module gon\n");
}

MODULE_INIT(init);
MODULE_EXIT(exit);
