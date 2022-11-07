#pragma once
#include <drivers/net/generic_card.h>
#include <net/arp.h>
#include <net/ip.h>
#include <types.h>

namespace net {
    class networkstack;
    namespace ethernet {
        class ethernet_stack;
    }
    namespace ip {
        class ipv4_stack;
    }
    namespace arp {
        class arp_stack;
    }
}

namespace net {
    class networkstack {
    public:
        networkstack(struct drivers::net::generic_card card);
        ~networkstack();

        void receive_packet(void *data, size_t size);

        net::ethernet::ethernet_stack *ethernet;
        net::ip::ipv4_stack *ipv4;
        net::arp::arp_stack *arp;

        struct drivers::net::generic_card networkcard;

    private:
    };
}
