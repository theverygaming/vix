#pragma once
#include <event.h>
#include <types.h>

namespace drivers::keyboard {
    extern event_dispatcher<char> events;
}
