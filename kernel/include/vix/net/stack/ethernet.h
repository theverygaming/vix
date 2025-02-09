#pragma once
#include <vix/net/stack/arp.h>
#include <vix/net/stack/ip.h>
#include <vix/types.h>
#include <vix/net/stack/packets.h>

namespace net {
    class networkstack;
}

namespace net {
    class ethernet {
    public:
        ethernet(uint8_t *mac_adr);

        void receive(net::networkstack *netstack, void *data, size_t size);
        /*
         * for the net::stack::packets::ethernet::header struct only the dest_mac and ethertype have to be set
         */
        void send(net::networkstack *netstack, struct net::stack::packets::ethernet::header *packet, void *data, size_t size);

        net::ipv4 ipv4;

        uint8_t mac[6];

    private:
    };
}
