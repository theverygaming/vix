#pragma once
#include <sched.h>

namespace sched {
    void init_proc(struct sched::proc *proc, void (*func)());
}
