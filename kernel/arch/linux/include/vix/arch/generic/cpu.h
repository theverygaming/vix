#pragma once

namespace arch::generic::cpu {
    /* this function must halt the cpu, disable all interrupts and never return */
    void halt();
}
