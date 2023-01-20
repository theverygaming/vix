#pragma once
#include <types.h>

namespace drivers::net::rtl8139 {
    void init();
    void sendPacket(void *data, size_t len);
    uint8_t get_mac_byte(int n);
}
