#include "mouse.h"
#include "christmas.h"
#include <vix/kprintf.h>
#include <vix/module.h>

MODULE_AUTHOR("theverygaming");
MODULE_DESCRIPTION("GUI experiments");
MODULE_VERSION("0.0.1");

static int init() {
    kprintf(KP_INFO, "guimod: loaded\n");
    mouse_init();
    christmas_init();
    return 0;
}

static void exit() {
    christmas_deinit();
    mouse_deinit();
    kprintf(KP_INFO, "guimod: unloaded\n");
}

MODULE_INIT(init);
MODULE_EXIT(exit);
