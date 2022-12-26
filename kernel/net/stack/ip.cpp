#include <endian.h>
#include <memory_alloc/memalloc.h>
#include <net/stack/icmp.h>
#include <net/stack/ip.h>
#include <net/stack/stack.h>
#include <stdio.h>
#include <stdlib.h>

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

static void print_ip(uint8_t *ip) {
    printf("%u.%u.%u.%u", (uint32_t)ip[0], (uint32_t)ip[1], (uint32_t)ip[2], (uint32_t)ip[3]);
}

void net::ipv4::receive(net::networkstack *netstack, void *data, size_t size) {
    printf("    -> ");
    struct ipv4_packet header;
    struct ipv4_packet *balls = (struct ipv4_packet *)data;
    header = *balls;

    for (int i = 0; i < 40; i++) {
        header.options[i] = 0;
    }
    header.data_start_offset = 21;
    for (int i = 0; i < 40; i++) {
        if (balls->options[i] == 0) {
            header.data_start_offset += +i;
            break;
        }
        printf("option: 0x%p, ", (uint32_t)balls->options[i]);
        header.options[i] = balls->options[i];
    }
    printf("src: ");
    print_ip(header.source_address);
    printf(" dst: ");
    print_ip(header.destination_address);

    header.data_start_offset = 21;

    struct ipv4_packet_processed processed_packet;
    for (int i = 0; i < 4; i++) {
        processed_packet.source_address[i] = header.source_address[i];
        processed_packet.destination_address[i] = header.destination_address[i];
    }
    processed_packet.protocol = header.protocol;

    void *dataptr = ((uint8_t *)data) + header.data_start_offset - 1;
    size_t packetsize = size - (header.data_start_offset - 1);

    if (header.protocol == 0x01) {
        printf(" ICMP\n");
        // netstack->icmp->receive_packet(netstack, &processed_packet, dataptr, packetsize);
        icmp.receive(netstack, dataptr, packetsize);
        return;
    }
    printf("\n");
}

void net::ipv4::send(net::networkstack *netstack, struct ipv4_packet_processed *packet, void *data, size_t size) {
    struct ipv4_packet packet_n;
    packet_n.version_ihl = 0x45;
    packet_n.TOS = 0x0;
    packet_n.total_length = BE_16((uint16_t)(size + 20));
    packet_n.identification = BE_16((uint16_t)0x81AF);        // only used for fragmentation/reassembly, we don't need it _for now_
    packet_n.flags_fragment_offset = BE_16((uint16_t)0x4000); // no fragmentation
    packet_n.TTL = 0x40;
    packet_n.protocol = packet->protocol;
    packet_n.header_checksum = 0x0000;
    memcpy(packet_n.source_address, ip, 4);
    memcpy(packet_n.destination_address, packet->destination_address, 4);

    // calculate checksum
    uint16_t *packet_u16 = (uint16_t *)&packet_n;
    uint16_t packet_sum = 0;
    for (int i = 0; i < 10; i++) {
        packet_sum += BE_16(packet_u16[i]);
    }
    packet_n.header_checksum = BE_16((uint16_t)(~packet_sum - 3)); // why -3? i have no clue -> maybe carry count???

    void *packetdata = memalloc::single::kmalloc(size + 20);
    memcpy(packetdata, &packet_n, 20);
    memcpy(((uint8_t *)packetdata) + 20, data, size);

    // quick hack
    if (known_ips.size() == 0) {
        known_ips.push_back({{192, 168, 56, 1}, {0xca, 0x85, 0xb4, 0x12, 0xc2, 0x34}});
    }

    int foundindex = -1;
    for (int i = 0; i < known_ips.size(); i++) {
        for (int j = 0; j < 4; j++) {
            if (known_ips[i].ip[j] != packet->destination_address[j]) {
                continue;
            }
        }
        foundindex = i;
        break;
    }
    if (foundindex == -1) {
        printf("could not find MAC address for IP\n");
        return;
    }

    struct net::ethernet::ethernet_packet_processed ethernetpacket;
    ethernetpacket.ethertype = 0x0008; // note: endianness
    for (uint8_t i = 0; i < 6; i++) {
        ethernetpacket.dest_mac[i] = known_ips[foundindex].mac[i];
    }

    netstack->ethernet->send(netstack, &ethernetpacket, packetdata, size + 20);

    memalloc::single::kfree(packetdata);
}
