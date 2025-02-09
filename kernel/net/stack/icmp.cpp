#include <stdlib.h>
#include <vix/endian.h>
#include <vix/kprintf.h>
#include <vix/net/stack/ethernet.h>
#include <vix/net/stack/icmp.h>
#include <vix/net/stack/ip.h>
#include <vix/net/stack/stack.h>

struct __attribute__((packed)) icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};

void net::icmp::receive(net::networkstack *netstack, struct net::ipv4_packet_processed *processed_packet, void *data, size_t size) {
    struct icmp_header *header = (struct icmp_header *)data;
    kprintf(KP_INFO, "        -> ICMP type: 0x%p\n", (uint32_t)header->type);
    header->type = 0;
    header->checksum += 0x8;

    struct net::ipv4_packet_processed packet;
    packet.protocol = 0x01;

    memcpy(packet.destination_address, processed_packet->source_address, 4);

    netstack->ethernet->ipv4.send(netstack, &packet, data, size);

    kprintf(KP_INFO, "        -> sent ICMP response\n");
}
