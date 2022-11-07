#pragma once
#include <net/ip.h>
#include <types.h>

namespace net {
    class networkstack;
    namespace ethernet {
        struct ethernet_packet_processed;
    }
    namespace ip {
        struct ipv4_packet_processed;
    }
}

namespace net::icmp {
    class icmp_stack {
    public:
        void receive_packet(net::networkstack *netstack, struct net::ethernet::ethernet_packet_processed *ethernet_packet, struct net::ip::ipv4_packet_processed *ipv4_packet, void *data, size_t size);

    private:
    };
}
