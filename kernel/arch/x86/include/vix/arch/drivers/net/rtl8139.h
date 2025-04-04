#pragma once
#include <vix/net/stack_rs/ethernet.h>
#include <vix/types.h>

namespace drivers::net::rtl8139 {
    bool sendPacket(struct ::net::ethernet_card *card, uint8_t *data, size_t len);
    uint8_t get_mac_byte(int n);
}
