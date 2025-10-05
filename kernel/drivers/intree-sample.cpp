#include <vix/kprintf.h>
#include <vix/module.h>
#include <vix/initcall.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("intree");
MODULE_VERSION("0.0.1");

void intree_link_test();

static int init() {
    kprintf(KP_INFO, "intree: loaded\n");
    intree_link_test();
    return 0;
}

static void exit() {
    kprintf(KP_INFO, "intree: unloaded\n");
}

MODULE_INIT(init);
MODULE_EXIT(exit);

// HACK: initcall when the module is in-tree
DEFINE_INITCALL(INITCALL_DRIVER_INIT, INITCALL_PRIO_NORMAL, init);
