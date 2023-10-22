#include <drivers/keyboard.h>
#include <types.h>

namespace drivers::keyboard {
    event_dispatcher<char> events;
}
