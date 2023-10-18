#pragma once
#include <sched.h>

namespace sched {
    void arch_init_proc(struct sched::task *proc, void (*func)());
}
