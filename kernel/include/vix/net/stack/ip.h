#pragma once
#include <vector>
#include <vix/net/stack/icmp.h>
#include <vix/types.h>

namespace net {
    class networkstack;
}

namespace net {
    struct ipv4_packet_processed {
        uint8_t source_address[4];
        uint8_t destination_address[4];
        uint8_t protocol;
    };

    class ipv4 {
    public:
        void receive(net::networkstack *netstack, void *data, size_t size);
        void send(net::networkstack *netstack, struct ipv4_packet_processed *packet, void *data, size_t size);

        uint8_t ip[4] = {192, 168, 69, 124};

        net::icmp icmp;

    private:
        struct knownip {
            uint8_t ip[4];
            uint8_t mac[6];
        };
        std::vector<struct knownip> known_ips; // std::map should be used here instead as soon as it is available
    };
}
