#include <vix/panic.h>
#include <vix/initfn.h>
#include <vix/kprintf.h>

extern "C" struct init_function START_INITFN_FUNCTIONS;
extern "C" struct init_function END_INITFN_FUNCTIONS;

void initfn_call(struct init_function *fn, unsigned int level) {
    // TODO: this should be an assertion instead sometime
    if (fn->level != level) {
        KERNEL_PANIC("initfn %s (level %u) called at level %u", fn->name, fn->level, level);
    }

    if (fn->flags & INIT_FUNCTION_FLAG_EXECUTED) {
        return;
    }
    for (size_t i = 0; i < fn->n_deps; i++) {
        initfn_call(fn->deps[i], level);
    }
    kprintf(KP_INFO, "initfn: %s\n", fn->name);
    fn->flags |= INIT_FUNCTION_FLAG_EXECUTED;
    if (fn->function != nullptr) {
        fn->function();
    }
}

void initfn_call_level(unsigned int level) {
    for (struct init_function *fn = &START_INITFN_FUNCTIONS; fn < &END_INITFN_FUNCTIONS; fn++) {
        if(fn->level == level) {
            initfn_call(fn, level);
        }
    }
}
