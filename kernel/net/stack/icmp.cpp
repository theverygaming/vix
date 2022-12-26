#include <arch/drivers/net/rtl8139.h>
#include <endian.h>
#include <net/stack/ethernet.h>
#include <net/stack/icmp.h>
#include <net/stack/ip.h>
#include <net/stack/stack.h>
#include <stdio.h>
#include <stdlib.h>

struct __attribute__((packed)) icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};

struct __attribute__((packed)) icmp_echo {
    uint16_t id;
    uint16_t seq;
};

void net::icmp::receive(net::networkstack *netstack, void *data, size_t size) {
    struct icmp_header *header = (struct icmp_header *)data;
    printf("        -> ICMP type: 0x%p\n", (uint32_t)header->type);
    header->type = 0;
    header->checksum += 0x8;

    struct net::ipv4::ipv4_packet_processed packet;
    packet.protocol = 0x01;

    memcpy(packet.destination_address, netstack->ethernet->ipv4.ip, 4);

    netstack->ethernet->ipv4.send(netstack, &packet, data, size);

    printf("        -> sent ICMP response\n");
}
