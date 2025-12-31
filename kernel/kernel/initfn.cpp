#include <vix/debug.h>
#include <vix/initfn.h>
#include <vix/kprintf.h>
#include <vix/panic.h>

extern "C" struct init_function *START_INITFN_FUNCTIONS;
extern "C" struct init_function *END_INITFN_FUNCTIONS;

void initfn_call(struct init_function *fn, unsigned int level) {
    DEBUG_PRINTF("initfn_call: ptr: 0x%p name: %s\n", fn, fn->name);

    if (fn->flags & INIT_FUNCTION_FLAG_EXECUTED) {
        return;
    }

    // TODO: this should be an assertion instead sometime
    // check below the already executed check as things from higher
    // levels are free to depend on lower levels if they want to.
    // It's just not allowed that something from a higher level causes
    // something from a lower level to be executed / the other way around
    if (fn->level != level) {
        KERNEL_PANIC("initfn %s (level %u) called at level %u", fn->name, fn->level, level);
    }

    for (size_t i = 0; i < fn->n_deps; i++) {
        initfn_call(*fn->deps[i], level);
    }
    kprintf(KP_INFO, "initfn: %s\n", fn->name);
    fn->flags |= INIT_FUNCTION_FLAG_EXECUTED;
    if (fn->function != nullptr) {
        fn->function();
    }
}

void initfn_call_level(unsigned int level) {
    kprintf(KP_INFO, "initfn: calling level %d\n", level);
    for (struct init_function **fni = &START_INITFN_FUNCTIONS; fni < &END_INITFN_FUNCTIONS; fni++) {
        struct init_function *fn = *fni;
        if(fn->level == level) {
            initfn_call(fn, level);
        }
    }
}
