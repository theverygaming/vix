#include <vix/kprintf.h>
#include <vix/module.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("module2");
MODULE_VERSION("0.0.1");

void module2_link_test();

static int init() {
    kprintf(KP_INFO, "module2: loaded\n");
    module2_link_test();
    return 0;
}

static void exit() {
    kprintf(KP_INFO, "module2: unloaded\n");
}

MODULE_INIT(init);
MODULE_EXIT(exit);
