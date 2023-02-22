#pragma once
#include <event.h>
#include <types.h>

namespace drivers::keyboard {
    void init();

    extern event_dispatcher<char> events;
}

namespace drivers::mouse {
    void init();
}
