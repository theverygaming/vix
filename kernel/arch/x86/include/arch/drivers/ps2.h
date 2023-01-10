#pragma once
#include <event.h>
#include <types.h>

namespace drivers::keyboard {
    void init();
    void poll();
    extern char buffer[100];
    extern int bufferlocation;

    extern event_dispatcher<char> events;
}

namespace drivers::mouse {
    void init();
}
