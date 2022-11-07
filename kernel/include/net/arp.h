#pragma once
#include <net/ethernet.h>
#include <net/stack.h>
#include <types.h>

namespace net {
    class networkstack;
    namespace ethernet {
        struct ethernet_packet_processed;
    }
}

namespace net::arp {
    class arp_stack {
    public:
        void receive_packet(net::networkstack *netstack, struct net::ethernet::ethernet_packet_processed *ethernet, void *data, size_t size);

    private:
    };
}
