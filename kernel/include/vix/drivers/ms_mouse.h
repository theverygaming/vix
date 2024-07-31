#pragma once
#include <vix/types.h>

namespace drivers::ms_mouse {
    void rx_packet(uint8_t *packet, uint8_t count);
    struct mouse_packet {
        int x_movement;
        int y_movement;
        bool lmb;
        bool rmb;
        bool mmb;
    };
    void register_event_listener(void (*event)(void *, struct mouse_packet), void *arg);
    void deregister_event_listener(void (*event)(void *, struct mouse_packet), void *arg);
}
