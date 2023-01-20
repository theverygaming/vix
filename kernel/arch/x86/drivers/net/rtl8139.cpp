#include <arch/cpubasics.h>
#include <arch/drivers/net/rtl8139.h>
#include <arch/drivers/pci.h>
#include <arch/drivers/pic_8259.h>
#include <arch/isr.h>
#include <arch/paging.h>
#include <drivers/net/generic_card.h>
#include <mm/kmalloc.h>
#include <net/stack/stack.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t bus;
static uint8_t device;
static uint8_t function;
static uint16_t io_base;
static uint8_t *bufferptr = nullptr; // uint8_t * because that makes code a bit cleaner
static uint16_t bufferoffset = 0;
static uint8_t irqline;

struct __attribute__((packed)) packetInfo {
    uint16_t header;
    uint16_t size;
};

static struct drivers::net::generic_card rtl8139_card = {
    .send_packet = drivers::net::rtl8139::sendPacket,
    .get_mac_byte = drivers::net::rtl8139::get_mac_byte,
};

static net::networkstack networkstack(rtl8139_card);

static void irq_handler(isr::registers *gaming) {
    if ((((struct packetInfo *)(bufferptr + bufferoffset))->header & 0x1) == 0) {
        printf("ROK not set?? wtf\n");
        KERNEL_PANIC("rtl8139 skill issue");
    }
    printf("rtl8139 IRQ\n");

    struct packet {
        void *ptr;
        size_t size;
    };

    std::vector<struct packet> received;

    // we must read packets in a loop until BUFE is set, could have received multiple
    while ((inb(io_base + 0x37) & 0x1) == 0) {
        uint16_t packetSize = ((struct packetInfo *)(bufferptr + bufferoffset))->size;
        // printf("size: %u\n", (uint32_t)packetSize);

        // net::ethernet::parse_ethernet_packet(((uint8_t *)(bufferptr + bufferoffset)) + sizeof(struct packetInfo), packetSize - 4); // 4 CRC bytes
        // networkstack.receive(((uint8_t *)(bufferptr + bufferoffset)) + sizeof(struct packetInfo), packetSize - 4); // 4 CRC bytes
        void *packet = mm::kmalloc(packetSize - 4);
        memcpy(packet, ((uint8_t *)(bufferptr + bufferoffset)) + sizeof(struct packetInfo), packetSize - 4);
        received.push_back({.ptr = packet, .size = packetSize - 4});

        // set new buffer offset
        bufferoffset = (bufferoffset + sizeof(packetInfo) + packetSize + 3) & ~0x3;

        // set CAPR to offset of next expected packet
        // printf("offset: %u\n", (uint32_t)bufferoffset);
        outw(io_base + 0x38, bufferoffset - 0x10);
        // TODO: handle buffer wrap
    }

    outw(io_base + 0x3E, 0x5); // clear RX ok bit

    printf("got %u packets\n", received.size());

    for (size_t i = 0; i < received.size(); i++) {
        networkstack.receive(received[i].ptr, received[i].size);
        mm::kfree(received[i].ptr);
    }

    drivers::pic::pic8259::eoi(drivers::pic::pic8259::irqToint(irqline));
}

static uint8_t reg_counter = 0;
void drivers::net::rtl8139::sendPacket(void *data, size_t len) {
    /* https://wiki.osdev.org/RTL8139
     * The RTL8139 NIC uses a round robin style for transmitting packets. It has four transmit buffer (a.k.a. transmit start) registers, and four transmit status/command registers. The transmit start
     * registers are each 32 bits long, and are in I/O offsets 0x20, 0x24, 0x28 and 0x2C. The transmit status/command registers are also each 32 bits long and are in I/O offsets 0x10, 0x14, 0x18 and
     * 0x1C. Each pair of transmit start and status registers work together (i.e. registers 0x20 and 0x10 work together, 0x24 and 0x14 work together, etc.)
     */
    if (reg_counter > 3) {
        reg_counter = 0;
    }
    outl(io_base + 0x20 + (reg_counter * 4), (uintptr_t)paging::get_physaddr_unaligned(data)); // transmit
    outl(io_base + 0x10 + (reg_counter * 4), len);                                             // status/command
    reg_counter++;
}

void drivers::net::rtl8139::init() {
    if (!drivers::pci::hasDev(0x10EC, 0x8139, &bus, &device, &function)) {
        printf("rtl8139n't :c\n");
        return;
    } else {
        printf("found rtl8139!\n");
    }

    if (drivers::pci::getBarType(bus, device, function, 0) != drivers::pci::bar_type::BAR_TYPE_IO) {
        printf("tf is wrong with this rtl8139?\n");
        return;
    }

    io_base = drivers::pci::getBarIOAddress(bus, device, function, 0);
    if (!io_base) {
        printf("couldn't get IO address for rtl8139\n");
        return;
    }

    if (drivers::pci::enableMastering(bus, device, function)) {
        printf("enabled mastering for rtl8139!\n");
    } else {
        return;
    }

    // reference: https://wiki.osdev.org/RTL8139

    // turn on the card
    outb(io_base + 0x52, 0x0);

    // soft reset
    outb(io_base + 0x37, 0x10);
    uint32_t timeout = 0;
    while ((inb(io_base + 0x37) & 0x10) != 0 && timeout < 0xFFFFFFF0) {
        timeout++;
    }

    printf("rtl8139 mac: ");
    for (int i = 0; i < 5; i++) {
        printf("%p:", (uint32_t)inb(io_base + 0x0 + i));
    }
    printf("%p\n", (uint32_t)inb(io_base + 0x5));

    // receive buffer
    bufferptr = (uint8_t *)mm::kmalloc(65536); // TODO: free
    if (((size_t)bufferptr) % 32) {
        printf("aligning buffer :troll:\n");
        bufferptr = bufferptr + (((size_t)bufferptr) % 4);
    }
    outl(io_base + 0x30, (uintptr_t)paging::get_physaddr_unaligned(bufferptr));

    // outw(io_base + 0x3C, 0x0005); // Sets the TOK and ROK bits high

    outw(io_base + 0x3C, 0x1); // set ROK bit high

    // outw(io_base + 0x3C, 0xE1FF); // enable all possible interrupts

    // configure receive buffer
    outl(io_base + 0x44, 0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

    // enable RX and TX
    outb(io_base + 0x37, 0x0C);
    irqline = drivers::pci::getInterruptLine(bus, device, function);
    printf("rtl8139 irq: %u\n", (uint32_t)irqline);

    isr::RegisterHandler(drivers::pic::pic8259::irqToint(irqline), irq_handler);
    drivers::pic::pic8259::unmask_irq(irqline);

    printf("rtl8139 init finished!\n");

    networkstack.init();
}

uint8_t drivers::net::rtl8139::get_mac_byte(int n) {
    if (n < 6) {
        return inb(io_base + 0x0 + n);
    }
    return 0;
}
