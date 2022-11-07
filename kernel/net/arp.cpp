#include <arch/x86/drivers/net/rtl8139.h>
#include <net/arp.h>
#include <stdio.h>
#include <stdlib.h>

struct __attribute__((packed)) arp_packet {
    uint16_t hardware_type; // usually 0x1 for Ethernet
    uint16_t protocol_type; // should be 0x0800 for IP
    uint8_t hardware_adr_len;
    uint8_t protocol_adr_len;
    uint16_t opcode;             // 0x01 for request, 0x02 for reply
    uint8_t src_hardware_adr[6]; // these sizes are assumed... A proper parser would never do this, but for testing it works i guess?
    uint8_t src_protocol_adr[4];
    uint8_t dest_hardware_adr[6];
    uint8_t dest_protocol_adr[4];
};

void net::arp::arp_stack::receive_packet(net::networkstack *netstack, struct net::ethernet::ethernet_packet_processed *ethernet, void *data, size_t size) {
    if (size < sizeof(struct arp_packet)) {
        printf("    -> packet too short for ARP\n");
        return;
    }
    struct arp_packet *packet = (struct arp_packet *)data;
    if (packet->opcode == 0x100) { // big endian...
        printf("    -> replying to ARP\n");
        struct arp_packet newpacket;
        memcpy(&newpacket, packet, sizeof(struct arp_packet));

        memcpy(newpacket.dest_protocol_adr, newpacket.src_protocol_adr, sizeof(newpacket.dest_protocol_adr));
        memcpy(newpacket.dest_hardware_adr, newpacket.src_hardware_adr, sizeof(newpacket.dest_hardware_adr));

        newpacket.opcode = 0x200; // reply

        memcpy(newpacket.src_protocol_adr, netstack->ipv4->ip, 4);

        for (uint8_t i = 0; i < 6; i++) {
            newpacket.src_hardware_adr[i] = netstack->ethernet->mac[i];
        }

        struct net::ethernet::ethernet_packet_processed ethernetpacket;
        ethernetpacket.ethertype = 0x0608;
        for (uint8_t i = 0; i < 6; i++) {
            ethernetpacket.dest_mac[i] = newpacket.dest_hardware_adr[i];
        }

        netstack->ethernet->send_packet(netstack, &ethernetpacket, &newpacket, sizeof(struct arp_packet));
    }
}
