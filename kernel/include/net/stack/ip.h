#pragma once
#include <net/stack/icmp.h>
#include <types.h>
#include <vector>

namespace net {
    class networkstack;
}

namespace net {
    class ipv4 {
    public:
        struct ipv4_packet_processed {
            uint8_t source_address[4];
            uint8_t destination_address[4];
            uint8_t protocol;
        };

        void receive(net::networkstack *netstack, void *data, size_t size);
        void send(net::networkstack *netstack, struct ipv4_packet_processed *packet, void *data, size_t size);

        uint8_t ip[4] = {192, 168, 56, 2};

        net::icmp icmp;

    private:
        struct knownip {
            uint8_t ip[4];
            uint8_t mac[6];
        };
        std::vector<struct knownip> known_ips; // std::map should be used here instead as soon as it is available
    };
}
