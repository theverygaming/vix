#include <arch/x86/drivers/net/rtl8139.h>
#include <endianness.h>
#include <net/ethernet.h>
#include <net/icmp.h>
#include <net/ip.h>
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

void net::icmp::icmp_stack::receive_packet(
    net::networkstack *netstack, struct net::ethernet::ethernet_packet_processed *ethernet_packet, struct net::ip::ipv4_packet_processed *ipv4_packet, void *data, size_t size) {
    struct icmp_header *header = (struct icmp_header *)data;
    printf("        -> ICMP type: 0x%p\n", (uint32_t)header->type);
    header->type = 0;
    header->checksum += 0x8;
    
    struct net::ip::ipv4_packet_processed packet;
    packet.protocol = ipv4_packet->protocol;

    memcpy(packet.destination_address, ipv4_packet->source_address, 4);
    
    netstack->ipv4->send_packet(netstack, &packet, data, size);

    printf("        -> sent ICMP response\n");
}
