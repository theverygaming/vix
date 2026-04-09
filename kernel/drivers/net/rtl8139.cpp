#include <vix/panic.h>
#include <forward_list>
#include <vix/kernel/io.h>
#include <vix/initfn.h>
#include <vix/mm/kheap.h>
#include <vix/net/stack_rs/ethernet.h>
#include <vix/sched.h>
#include <vix/drivers/pci.h>
#include <vix/kernel/irq.h>

#define RX_BUFFER_SIZE_IDX 1
#define RX_BUFFER_SIZE     (8192 << RX_BUFFER_SIZE_IDX)

struct rtl8139_card {
    // IO stuff
    struct pci::pci_dev *pcidev;
    io_handle_t iohandle;
    uint8_t irqline;

    // external references
    struct net::ethernet_card *card;

    // internal state
    uint8_t *bufferptr;
    uint16_t bufferoffset;
    uint8_t reg_counter;
};

std::forward_list<struct rtl8139_card *> rtl8139_instances;

struct __attribute__((packed)) packetInfo {
    uint16_t header;
    uint16_t size;
};

static mm::paddr_t get_physaddr_unaligned(mm::vaddr_t virt) {
    size_t diff = ALIGN_DOWN_DIFF(virt, CONFIG_ARCH_PAGE_SIZE);
    mm::paddr_t addr = arch::vmm::get_page(ALIGN_DOWN(virt, CONFIG_ARCH_PAGE_SIZE), nullptr);
    return addr + diff;
}

static uint8_t get_mac_byte(struct rtl8139_card *ctx, int n) {
    if (n < 6) {
        return ioread8(ctx->iohandle + 0x0 + n);
    }
    return 0;
}

static bool send_packet(struct ::net::ethernet_card *card, uint8_t *data, size_t len) {
    struct rtl8139_card *ctx = (struct rtl8139_card *)card->ctx;
    /* https://wiki.osdev.org/RTL8139
     * The RTL8139 NIC uses a round robin style for transmitting packets. It has four transmit buffer (a.k.a. transmit start) registers, and four transmit status/command registers. The transmit start
     * registers are each 32 bits long, and are in I/O offsets 0x20, 0x24, 0x28 and 0x2C. The transmit status/command registers are also each 32 bits long and are in I/O offsets 0x10, 0x14, 0x18 and
     * 0x1C. Each pair of transmit start and status registers work together (i.e. registers 0x20 and 0x10 work together, 0x24 and 0x14 work together, etc.)
     */
    if (ctx->reg_counter > 3) {
        ctx->reg_counter = 0;
    }
    iowrite32(ctx->iohandle + 0x20 + (ctx->reg_counter * 4), get_physaddr_unaligned((mm::vaddr_t)data)); // transmit
    iowrite32(ctx->iohandle + 0x10 + (ctx->reg_counter * 4), len);                                            // status/command
    ctx->reg_counter++;
    return true;
}

static struct net::ethernet_card_ops rtl8139_ops = {
    .transmit = send_packet,
    .get_mac =
        [](struct net::ethernet_card *card, uint8_t *mac, size_t len) {
            struct rtl8139_card *ctx = (struct rtl8139_card *)card->ctx;
            if (len != 6) {
                return false;
            }
            for (int i = 0; i < 6; i++) {
                mac[i] = get_mac_byte(ctx, i);
            }
            return true;
        },
};

static void rtl8139_irq(struct rtl8139_card *ctx) {
    if ((((struct packetInfo *)(ctx->bufferptr + ctx->bufferoffset))->header & 0x1) == 0) {
        //KERNEL_PANIC("ROK not set?? wtf - rtl8139 skill issue");
        // FIXME: skill issue fr?
        DEBUG_PRINTF("rtl8139 IRQ -- ROK not set??\n");
        return;
    }
    DEBUG_PRINTF("rtl8139 IRQ\n");

    struct packet {
        void *ptr;
        size_t size;
    };

    std::vector<struct packet> received;

    // we must read packets in a loop until BUFE is set, could have received multiple
    while ((ioread8(ctx->iohandle + 0x37) & 0x1) == 0) {

        DEBUG_PRINTF("Buffer offset: %u\n", ctx->bufferoffset);

        // FIXME: i can imagine a bit of edge case where this would break when we have a buffer wrap!
        if (ctx->bufferoffset == RX_BUFFER_SIZE) {
            DEBUG_PRINTF("EDGE CASE");
        }
        if (ctx->bufferoffset > RX_BUFFER_SIZE) {
            DEBUG_PRINTF("EDGE CASE2");
        }
        if (ctx->bufferoffset == RX_BUFFER_SIZE) {
            ctx->bufferoffset = 0;
        }
        uint16_t packetSize = ((struct packetInfo *)(ctx->bufferptr + ctx->bufferoffset))->size;
        uint16_t packetSizeNoCRC = packetSize - 4;

        // these cases should be IMPOSSIBLE and if they do happen we're fucked
        if (packetSize <= 4 || packetSize > RX_BUFFER_SIZE) {
            KERNEL_PANIC("WTF");
        }

        void *packet = mm::kmalloc(packetSizeNoCRC);

        DEBUG_PRINTF("Buffer offset: %u Packet size: %u\n", ctx->bufferoffset, packetSize);

        // buffer wrap
        if ((ctx->bufferoffset + sizeof(struct packetInfo) + packetSize) > RX_BUFFER_SIZE) {
            size_t first_split = RX_BUFFER_SIZE - (ctx->bufferoffset + sizeof(struct packetInfo));

            size_t n_read1 = first_split;
            size_t n_read2 = (packetSizeNoCRC)-first_split;

            // another special case: only the CRC is what is remaining after the buffer wrapped
            if ((packetSize - first_split) <= 4) {
                // this could mean the CRC would be contained in the first read and go beyond the buffer, we sure don't want that!
                n_read1 = packetSizeNoCRC;
                // also n_read_2 will be some negative value so we set it to zero
                n_read2 = 0;
            }

            DEBUG_PRINTF("BUFFER WRAP split: %u!\n", first_split);

            memcpy(packet, ((uint8_t *)(ctx->bufferptr + ctx->bufferoffset)) + sizeof(struct packetInfo), n_read1);

            ctx->bufferoffset = 0;

            memcpy((uint8_t *)packet + first_split, (uint8_t *)(ctx->bufferptr + ctx->bufferoffset), n_read2);

            // set new buffer offset
            ctx->bufferoffset = ALIGN_UP(ctx->bufferoffset + (packetSize - first_split), 4);
            DEBUG_PRINTF("wbufferoffset %u\n", ctx->bufferoffset);
        } else {
            DEBUG_PRINTF("nbufferoffset1 %u\n", ctx->bufferoffset);
            memcpy(packet, ((uint8_t *)(ctx->bufferptr + ctx->bufferoffset)) + sizeof(struct packetInfo),
                   packetSizeNoCRC); // 4 CRC bytes
            // set new buffer offset
            ctx->bufferoffset = ALIGN_UP((ctx->bufferoffset + sizeof(packetInfo) + packetSize), 4);
            DEBUG_PRINTF("nbufferoffset2 %u\n", ctx->bufferoffset);
        }
        received.push_back({.ptr = packet, .size = packetSizeNoCRC});

        // set CAPR to offset of next expected packet
        // DEBUG_PRINTF("offset: %u\n", (uint32_t)bufferoffset);
        iowrite16(ctx->iohandle + 0x38, ctx->bufferoffset - 0x10);
    }

    iowrite16(ctx->iohandle + 0x3E, 0x5); // clear RX ok bit

    DEBUG_PRINTF("got %u packets\n", received.size());

    for (size_t i = 0; i < received.size(); i++) {
        netstack_ethernet_rx(ctx->card, (uint8_t *)received[i].ptr, received[i].size);
        mm::kfree(received[i].ptr);
    }
}

// FIXME: fix IRQs
static rtl8139_card *irq_card = nullptr;

static void irq_handler() {
    rtl8139_irq(irq_card);
}

static void rtl8139_init(struct rtl8139_card *ctx) {
    // reference: https://wiki.osdev.org/RTL8139

    pci::pci_dev_master(ctx->pcidev, true);

    // turn on the card
    iowrite8(ctx->iohandle + 0x52, 0x0);

    // soft reset
    iowrite8(ctx->iohandle + 0x37, 0x10);
    uint32_t timeout = 0;
    while ((ioread8(ctx->iohandle + 0x37) & 0x10) != 0 && timeout < 0xFFFFFFF0) {
        timeout++;
    }

    DEBUG_PRINTF("rtl8139 mac:\n");
    for (int i = 0; i < 5; i++) {
        DEBUG_PRINTF("%p:\n", (uint32_t)ioread8(ctx->iohandle + 0x0 + i));
    }
    DEBUG_PRINTF("%p\n", (uint32_t)ioread8(ctx->iohandle + 0x5));

    // receive buffer
    // rx buf len RX_BUFFER_SIZE + 16 byte
    void *tmp_alloc;
    ASSIGN_OR_PANIC(tmp_alloc, mm::allocate_contiguous(RX_BUFFER_SIZE + 16)); // TODO: free
    ctx->bufferptr = (uint8_t *)tmp_alloc;
    iowrite32(ctx->iohandle + 0x30, get_physaddr_unaligned((mm::vaddr_t)ctx->bufferptr));

    // iowrite16(ctx->iohandle + 0x3C, 0x0005); // Sets the TOK and ROK bits high

    iowrite16(ctx->iohandle + 0x3C, 0x1); // set ROK bit high

    // iowrite16(ctx->iohandle + 0x3C, 0xE1FF); // enable all possible interrupts

    // configure receive buffer
    iowrite32(ctx->iohandle + 0x44, 0xf | (0 << 7) | (RX_BUFFER_SIZE_IDX << 11)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

    // enable RX and TX
    iowrite8(ctx->iohandle + 0x37, 0x0C);
    ctx->irqline = pci::pci_dev_get_irqline(ctx->pcidev);
    DEBUG_PRINTF("rtl8139 irq: %u\n", (uint32_t)ctx->irqline);

    if (irq_card == nullptr) {
        irq_card = ctx;
    } else {
        KERNEL_PANIC("more than one rtl8139 not supported. The IRQ implementation would have to be fixed for that");
    }

    irq::register_irq_handler(irq_handler, ctx->irqline);

    DEBUG_PRINTF("rtl8139 init finished!... registering card\n");

    ctx->card = netstack_ethernet_register_card(&rtl8139_ops);
    ctx->card->ctx = ctx;

    DEBUG_PRINTF("rtl8139 registered!\n");
}

static bool rtl8139_probe(struct pci::pci_dev *dev) {
    struct rtl8139_card *card = new rtl8139_card {
        // IO stuff
        .pcidev = dev,
        .iohandle = pci_bar_iomap(pci::pci_dev_get_bar(dev, 0)),
        .irqline = 0,
        .card = nullptr,
        .bufferptr = nullptr,
        .bufferoffset = 0,
        .reg_counter = 0,
    };
    rtl8139_instances.push_front(card);

    rtl8139_init(card);

    return true;
}

struct pci::pci_driver rtl8139_pci = {
    .name = "rtl8139",
    .vendor_id = 0x10EC,
    .device_id = 0x8139,
    .probe = rtl8139_probe,
};

static void rtl8139_driver_init() {
    pci::register_driver(&rtl8139_pci);
}

INITFN_DEFINE(rtl8139_driver_init, INITFN_DRIVER_INIT, 0, rtl8139_driver_init);
