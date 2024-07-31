#pragma once
#include <vix/drivers/net/generic_card.h>
#include <vix/net/stack/ethernet.h>
#include <vix/types.h>

namespace net {
    class networkstack {
    public:
        networkstack(struct drivers::net::generic_card card);
        ~networkstack();

        void init();

        net::ethernet *ethernet;

        /* data may be modified */
        void receive(void *data, size_t size);

        struct drivers::net::generic_card networkcard;

    private:
        bool _init = false;
    };
}
