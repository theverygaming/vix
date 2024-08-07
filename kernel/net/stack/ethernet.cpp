#include <stdlib.h>
#include <vix/endian.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/net/stack/arp.h>
#include <vix/net/stack/ethernet.h>
#include <vix/net/stack/ip.h>
#include <vix/net/stack/stack.h>

#define ETHERTYPE_IPV4  0x800
#define ETHERTYPE_IPV6  0x86DD
#define ETHERTYPE_ARP   0x806
#define ETHERTYPE_FRARP 0x808

// TODO: 802.1Q support
struct __attribute__((packed)) ethernet_packet {
    uint8_t dest_mac[6];
    uint8_t source_mac[6];
    uint16_t ethertype;
};

static void print_mac(uint8_t *mac) {
    for (int i = 0; i < 5; i++) {
        kprintf(KP_INFO, "%p:", (uint32_t)mac[i]);
    }
    kprintf(KP_INFO, "%p", (uint32_t)mac[5]);
}

net::ethernet::ethernet(uint8_t *mac_adr) {
    memcpy(mac, mac_adr, 6);
}

void net::ethernet::receive(net::networkstack *netstack, void *data, size_t size) {
    if (size < sizeof(struct ethernet_packet)) {
        return;
    }
    struct ethernet_packet *packet = (struct ethernet_packet *)data;

    packet->ethertype = BE_16(packet->ethertype);
    // if ethertype 0000-05DC it's length

    kprintf(KP_INFO, "src: ");
    print_mac(packet->source_mac);
    kprintf(KP_INFO, " dst: ");
    print_mac(packet->dest_mac);

    struct ethernet_packet_processed processed_packet;
    for (int i = 0; i < 6; i++) {
        processed_packet.dest_mac[i] = packet->dest_mac[i];
        processed_packet.source_mac[i] = packet->source_mac[i];
    }
    processed_packet.ethertype = packet->ethertype;

    void *dataptr = ((uint8_t *)data) + sizeof(struct ethernet_packet);
    size_t packetsize = size - sizeof(struct ethernet_packet);

    switch (packet->ethertype) {
    case ETHERTYPE_IPV4:
        kprintf(KP_INFO, "-> ipv4\n");
        ipv4.receive(netstack, dataptr, packetsize);
        break;
    case ETHERTYPE_IPV6:
        kprintf(KP_INFO, "-> ipv6\n");
        break;
    case ETHERTYPE_ARP:
        kprintf(KP_INFO, "-> ARP\n");
        arp.receive(netstack, dataptr, packetsize);
        break;
    case ETHERTYPE_FRARP:
        kprintf(KP_INFO, "-> FRARP\n");
        break;
    default:
        kprintf(KP_INFO, " -> 0x%p(???)\n", (uint32_t)packet->ethertype);
        break;
    }
}

void net::ethernet::send(net::networkstack *netstack, struct ethernet_packet_processed *packet, void *data, size_t size) {
    void *new_data = mm::kmalloc(size + sizeof(struct ethernet_packet));
    memcpy(((uint8_t *)new_data) + sizeof(struct ethernet_packet), data, size);
    struct ethernet_packet *ethernetpacket = (struct ethernet_packet *)new_data;

    for (int i = 0; i < 6; i++) {
        ethernetpacket->dest_mac[i] = packet->dest_mac[i];
        ethernetpacket->source_mac[i] = mac[i];
    }
    ethernetpacket->ethertype = packet->ethertype;

    netstack->networkcard.send_packet(new_data, size + sizeof(struct ethernet_packet));
    mm::kfree(new_data);
}
