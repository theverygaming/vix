#include "ethernet.h"
#include <endianness.h>
#include <net/ip.h>
#include <stdio.h>

#define ETHERTYPE_IPV4 0x800
#define ETHERTYPE_IPV6 0x86DD
#define ETHERTYPE_ARP 0x806
#define ETHERTYPE_FRARP 0x808

void print_mac(uint8_t *mac) {
    printf("%p:%p:%p:%p:%p:%p\n", (uint32_t)mac[0], (uint32_t)mac[1], (uint32_t)mac[2], (uint32_t)mac[3], (uint32_t)mac[4], (uint32_t)mac[5]);
}

bool net::ethernet::parse_ethernet_packet(void *data, size_t len) {
    if (len < sizeof(ethernet_packet_t)) {
        return false;
    }
    ethernet_packet_t *packet = (ethernet_packet_t *)data;

    packet->ethertype = endian_assure_big(packet->ethertype);
    // if ethertype 0000-05DC it's length

    printf("src: ");
    print_mac(packet->source_mac);
    printf("dst: ");
    print_mac(packet->dest_mac);
    printf("type: 0x%p\n", (uint32_t)packet->ethertype);

    switch (packet->ethertype) {
    case ETHERTYPE_IPV4:
        printf("ipv4\n");
        net::ip::parse_ipv4_packet(((uint8_t *)data) + sizeof(ethernet_packet_t), len - sizeof(ethernet_packet_t));
        break;
    case ETHERTYPE_IPV6:
        printf("ipv6\n");
        break;
    case ETHERTYPE_ARP:
        printf("ARP\n");
        break;
    case ETHERTYPE_FRARP:
        printf("FRARP\n");
        break;
    default:
        break;
    }

    return true;
}