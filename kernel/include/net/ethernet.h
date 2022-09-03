#pragma once
#include <types.h>

namespace net::ethernet {
    // TODO: 802.1Q support
    typedef struct __attribute__((packed)) {
        uint8_t dest_mac[6];
        uint8_t source_mac[6];
        uint16_t ethertype;
    } ethernet_packet_t;
    bool parse_ethernet_packet(void *data, size_t len);
}
