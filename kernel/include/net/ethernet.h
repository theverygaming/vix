#pragma once
#include <net/arp.h>
#include <net/ip.h>
#include <types.h>

namespace net {
    class networkstack;
}

namespace net {
    class ethernet {
    public:
        struct ethernet_packet_processed {
            uint8_t dest_mac[6];
            uint8_t source_mac[6];
            uint16_t ethertype; // must be in the architectures endianness
        };

        ethernet(uint8_t *mac_adr);

        void receive(net::networkstack *netstack, void *data, size_t size);
        /*
         * for the ethernet_packet_processed struct only the dest_mac and ethertype have to be set
         */
        void send(net::networkstack *netstack, struct ethernet_packet_processed *packet, void *data, size_t size);

        net::ipv4 ipv4;
        net::arp arp;

        uint8_t mac[6];

    private:
    };
}
