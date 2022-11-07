#pragma once
#include <net/stack.h>
#include <types.h>

namespace net {
    class networkstack;
}

namespace net::ethernet {
    class ethernet_stack {
    public:
        void receive_packet(net::networkstack *netstack, void *data, size_t size);

        /*
         * for the ethernet_packet_processed struct only the dest_mac and ethertype have to be set
         */
        void send_packet(net::networkstack *netstack, struct ethernet_packet_processed *packet, void *data, size_t size);

        uint8_t mac[6];

    private:
    };

    // TODO: 802.1Q support
    struct __attribute__((packed)) ethernet_packet {
        uint8_t dest_mac[6];
        uint8_t source_mac[6];
        uint16_t ethertype;
    };

    struct ethernet_packet_processed {
        uint8_t dest_mac[6];
        uint8_t source_mac[6];
        uint16_t ethertype; // must be in the architectures endianness
    };
}
