#pragma once
#include <vix/types.h>

namespace net {
    class networkstack;
    struct ipv4_packet_processed;
}

namespace net {
    class icmp {
    public:
        void receive(net::networkstack *netstack, struct net::ipv4_packet_processed *processed_packet, void *data, size_t size);
    };
}
