#pragma once
#include <cppstd/vector.h>
#include <net/ethernet.h>
#include <net/icmp.h>
#include <net/stack.h>
#include <types.h>

namespace net {
    class networkstack;
    namespace ethernet {
        struct ethernet_packet_processed;
    }
    namespace icmp {
        class icmp_stack;
    }
}

namespace net::ip {
    struct ipv4_packet_processed {
        uint8_t source_address[4];
        uint8_t destination_address[4];
        uint8_t protocol;
    };

    class ipv4_stack {
    public:
        void receive_packet(net::networkstack *netstack, struct net::ethernet::ethernet_packet_processed *ethernet_packet, void *data, size_t size);

        /*
         * for the ipv4_packet_processed struct only the destination address and protocol have to be set
         */
        void send_packet(net::networkstack *netstack, struct ipv4_packet_processed *packet, void *data, size_t size);

        uint8_t ip[4] = {192, 168, 56, 2};

        net::icmp::icmp_stack *icmp;

    private:
        struct knownip {
            uint8_t ip[4];
            uint8_t mac[6];
        };
        std::vector<struct knownip> known_ips; // std::map should be used here instead as soon as it is available
    };

    struct __attribute__((packed)) ipv4_packet {
        uint8_t version_ihl;
        uint8_t TOS;
        uint16_t total_length;
        uint16_t identification;
        uint16_t flags_fragment_offset;
        uint8_t TTL;
        uint8_t protocol;
        uint16_t header_checksum;
        uint8_t source_address[4];
        uint8_t destination_address[4];

        uint8_t options[40];

        uint32_t data_start_offset;
    };

    void parse_ipv4_packet(void *data, size_t len);
    void *
    make_ipv4_packet(const uint8_t *src_address, const uint8_t *dest_address, uint16_t payload_length, uint8_t protocol, size_t *output_len); // returns memory from new[], must be freed using delete[]
}
