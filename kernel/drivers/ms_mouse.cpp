#include <drivers/ms_mouse.h>
#include <panic.h>
#include <stdio.h>
#include <vector>

struct event_listener {
    void (*event)(void *, struct drivers::ms_mouse::mouse_packet);
    void *arg;
};

static std::vector<struct event_listener> listeners;

void drivers::ms_mouse::rx_packet(uint8_t *packet, uint8_t count) {
    // printf("got mouse packet!\n");
    if (count < 3) { // stupid check, remove
        return;
    }
    if (packet[0] & 0x80 || packet[0] & 0x40) {
        // printf("trash packet\n");
        return;
    }
    struct mouse_packet packet_s = {
        .x_movement = 0,
        .y_movement = 0,
        .lmb = (packet[0] & 0x1) != 0,
        .rmb = (packet[0] & 0x2) != 0,
        .mmb = (packet[0] & 0x4) != 0,
    };

    if (packet[0] & 0x1) {
        // printf("LMB ");
    }

    if (packet[0] & 0x2) {
        // printf("RMB ");
    }

    if (packet[0] & 0x4) {
        // printf("MMB ");
    }

    int32_t x = packet[1];
    int32_t y = packet[2];

    if (packet[0] & 0x20) {
        y |= 0xFFFFFF00;
    }
    if (packet[0] & 0x10) {
        x |= 0xFFFFFF00;
    }

    packet_s.x_movement = x;
    packet_s.y_movement = y;

    // printf("x = %d, y = %d\n", x, y);

    for (size_t i = 0; i < listeners.size(); i++) {
        listeners[i].event(listeners[i].arg, packet_s);
    }
}

void drivers::ms_mouse::register_event_listener(void (*event)(void *, struct mouse_packet), void *arg) {
    for (size_t i = 0; i < listeners.size(); i++) {
        if (listeners[i].event == event && listeners[i].arg == arg) {
            return;
        }
    }
    listeners.push_back({.event = event, .arg = arg});
}

void drivers::ms_mouse::deregister_event_listener(void (*event)(void *, struct mouse_packet), void *arg) {
    for (size_t i = 0; i < listeners.size(); i++) {
        if (listeners[i].event == event && listeners[i].arg == arg) {
            listeners.erase(i);
            break;
        }
    }
}
