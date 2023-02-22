#pragma once
#include <event.h>
#include <types.h>

namespace drivers::keyboard {
    typedef uint8_t keycode_t;
    void init();

    extern event_dispatcher<char> events;
    extern event_dispatcher<uint8_t> raw_press_events;
    extern event_dispatcher<uint8_t> raw_release_events;
}

namespace drivers::mouse {
    void init();
}
