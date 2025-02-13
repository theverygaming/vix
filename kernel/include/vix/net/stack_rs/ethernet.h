#pragma once
#include <vix/types.h>

namespace net {
    struct ethernet_card_ops {
        bool (*transmit)(struct ethernet_card *card, uint8_t *buf, size_t size);
        bool (*get_mac)(struct ethernet_card *card, uint8_t *mac, size_t size);
    };

    struct ethernet_card {
        struct ethernet_card_ops *ops;
        void *ctx;
    };
}

extern "C" struct net::ethernet_card *netstack_ethernet_register_card(struct net::ethernet_card_ops *ops);
extern "C" void netstack_ethernet_rx(struct net::ethernet_card *card, uint8_t *buf, size_t size);
