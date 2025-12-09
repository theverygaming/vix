#include <vix/kprintf.h>
#include <vix/module.h>
#include <vix/initfn.h>

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

static void init_intree() {
    init();
}

// HACK: initfn when the module is in-tree
INITFN_DEFINE(intree, INITFN_DRIVER_INIT, 0, init_intree);
