#include <vix/initcall.h>
#include <vix/kprintf.h>

extern "C" initcall_t START_INITCALL_LEVEL_0;
extern "C" initcall_t END_INITCALL_LEVEL_0;
extern "C" initcall_t START_INITCALL_LEVEL_1;
extern "C" initcall_t END_INITCALL_LEVEL_1;

void initcall_init_level(int level) {
    if (level < 0 || level > 1) {
        return;
    }
    kprintf(KP_INFO, "initcall: calling level %d\n", level);
    initcall_t *levels[] = {
        &START_INITCALL_LEVEL_0,
        &END_INITCALL_LEVEL_0,
        &START_INITCALL_LEVEL_1,
        &END_INITCALL_LEVEL_1,
    };

    for (initcall_t *i = levels[level * 2]; i < levels[(level * 2) + 1]; i++) {
        int ret = (*i)();
        if (ret != 0) {
            kprintf(KP_WARNING, "Initcall function 0x%p returned %d\n", (uintptr_t)*i, ret);
        };
    }
}
