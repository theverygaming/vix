#pragma once
#include <types.h>

namespace drivers::net::rtl8139 {
    void init();
    void sendPacket(void *data, uint32_t len);
}