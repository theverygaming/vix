#include <stdlib.h>
#include <vix/endian.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/net/stack/arp.h>
#include <vix/net/stack/ethernet.h>
#include <vix/net/stack/ip.h>
#include <vix/net/stack/packets.h>
#include <vix/net/stack/stack.h>

using namespace net::stack;

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
    if (size < sizeof(struct packets::ethernet::header)) {
        return;
    }
    struct packets::ethernet::header *packet = (struct packets::ethernet::header *)data;

    packet->swap();
    // if ethertype 0000-05DC it's length

    kprintf(KP_INFO, "src: ");
    print_mac(packet->source_mac);
    kprintf(KP_INFO, " dst: ");
    print_mac(packet->dest_mac);

    void *dataptr = ((uint8_t *)data) + sizeof(struct packets::ethernet::header);
    size_t packetsize = size - sizeof(struct packets::ethernet::header);

    switch (packet->ethertype) {
    case packets::ethernet::ETHERTYPE_IPV4:
        kprintf(KP_INFO, "-> ipv4\n");
        ipv4.receive(netstack, dataptr, packetsize);
        break;
    case packets::ethernet::ETHERTYPE_IPV6:
        kprintf(KP_INFO, "-> ipv6\n");
        break;
    case packets::ethernet::ETHERTYPE_ARP:
        kprintf(KP_INFO, "-> ARP\n");
        net::arp::receive(netstack, dataptr, packetsize);
        break;
    default:
        kprintf(KP_INFO, " -> 0x%p(???)\n", (uint32_t)packet->ethertype);
        break;
    }
}

void net::ethernet::send(net::networkstack *netstack, struct packets::ethernet::header *packet, void *data, size_t size) {
    void *new_data = mm::kmalloc(size + sizeof(struct packets::ethernet::header));
    memcpy(((uint8_t *)new_data) + sizeof(struct packets::ethernet::header), data, size);
    struct packets::ethernet::header *ethernetpacket = (struct packets::ethernet::header *)new_data;

    for (int i = 0; i < 6; i++) {
        ethernetpacket->dest_mac[i] = packet->dest_mac[i];
        ethernetpacket->source_mac[i] = mac[i];
    }
    ethernetpacket->ethertype = packet->ethertype;

    netstack->networkcard.send_packet(new_data, size + sizeof(struct packets::ethernet::header));
    mm::kfree(new_data);
}
