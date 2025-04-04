#include <vector>
#include <vix/initcall.h>
#include <vix/kprintf.h>
#include <vix/mm/kheap.h>
#include <vix/net/stack_rs/ethernet.h>
#include <vix/sched.h>

// clang-format off
static const uint8_t test_packet_0[] = {
    // destination MAC
    'M', 'A', 'C', 'D', 'S', 'T',
    // source MAC
    'M', 'A', 'C', 'S', 'R', 'C',
    // ethertype (BE)
    0x00, 0x01,
    // some dummy data
    'T', 'E', 'S', 'T', ' ', 'D', 'A', 'T', 'A',
    'T', 'E', 'S', 'T', ' ', 'D', 'A', 'T', 'A',
    'T', 'E', 'S', 'T', ' ', 'D', 'A', 'T', 'A',
    'T', 'E', 'S', 'T', ' ', 'D', 'A', 'T', 'A',
    'T', 'E', 'S', 'T', ' ', 'D', 'A', 'T', 'A',
};

static const uint8_t test_packet_1[] = {
    // destination MAC
    'M', 'A', 'C', 'D', 'S', 'T',
    // source MAC
    'M', 'A', 'C', 'S', 'R', 'C',
    // ethertype (BE) -- IPv4 0x0800
    0x08, 0x00,
    // IPv4:
    // Version: 4, 20-byte header
    0x45,
    // Differential services: 0
    0x00,
    // Total Length (IPv4 Header + Data)
    0x00, 20 + 12, // FIXME:
    // Identification - used in case of fragmented packets
    0x12, 0x34,
    // Flags & Fragment offset
    0x00 | 0b010, 0x00,
    // TTL
    69,
    // Next layer protocol
    17, // UDP
    // Checksum
    0x00, 0x00, // FIXME:
    // Source address
    111, 222, 233, 244,
    // Destination address
    255, 206, 207, 208,
    // nonsense data
    'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', '!'
};
// clang-format on

static const struct {
    const uint8_t *ptr;
    size_t size;
} test_packets[] = {
    {test_packet_0, sizeof(test_packet_0)},
    {test_packet_1, sizeof(test_packet_1)},
};

static bool send_packet(struct ::net::ethernet_card *card, uint8_t *data, size_t len) {
    DEBUG_PRINTF("send_packet 0x%p %u\n", data, len);
    return true;
}

static struct net::ethernet_card_ops fake_card_ops = {
    .transmit = send_packet,
    .get_mac =
        [](struct net::ethernet_card *card, uint8_t *mac, size_t len) {
            if (len != 6) {
                return false;
            }
            for (int i = 0; i < 6; i++) {
                mac[i] = 0x10 + i;
            }
            return true;
        },
};

// FIXME: this is ofc bad lmao, but we currently don't really have another way
// without writing EVEN MORE CODE MOSTLY UNRELAYED TO WHAT I WANNA DO :sob:
static net::ethernet_card *current_card = nullptr;

static void rx_fake_packets(int n_packets) {
    struct packet {
        void *ptr;
        size_t size;
    };

    int max_n_packets = sizeof(test_packets) / sizeof(test_packets[0]);
    if (n_packets > max_n_packets) {
        n_packets = max_n_packets;
    }

    std::vector<struct packet> received;

    for (int i = 0; i < n_packets; i++) {
        void *packet = mm::kmalloc(test_packets[i].size);
        memcpy(packet, test_packets[i].ptr, test_packets[i].size);
        received.push_back({.ptr = packet, .size = test_packets[i].size});
    }

    DEBUG_PRINTF("got %u packets\n", n_packets);

    for (size_t i = 0; i < received.size(); i++) {
        DEBUG_PRINTF("providing packet %u to network stack (0x%p, %u)\n", i, received[i].ptr, received[i].size);
        netstack_ethernet_rx(current_card, (uint8_t *)received[i].ptr, received[i].size);
        mm::kfree(received[i].ptr);
    }
}

static void fake_packet_gen() {
    uint32_t counter_max = 60000;
    uint32_t counter = counter_max / 2;
    while (true) {
        if (counter == 0) {
            rx_fake_packets(2);
        }
        counter++;
        if (counter > counter_max) {
            counter = 0;
        }
        sched::yield();
    }
}

static int fake_card_init() {
    kprintf(KP_INFO, "registering fake_card\n");
    current_card = netstack_ethernet_register_card(&fake_card_ops);
    kprintf(KP_INFO, "fake_card registered!\n");

    sched::start_thread(fake_packet_gen);

    kprintf(KP_INFO, "fake_card done initializing\n");

    return 0;
}

DEFINE_INITCALL(INITCALL_FIRST_THREAD, fake_card_init);
