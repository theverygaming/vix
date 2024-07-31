#pragma once
#include <vix/event.h>
#include <vix/types.h>

namespace drivers::keyboard {
    extern event_dispatcher<char> events;
}
