#include <string.h>
#include <vix/arch/common/cpu.h>
#include <vix/arch/cpubasics.h>
#include <vix/arch/drivers/net/rtl8139.h>
#include <vix/arch/drivers/pci.h>
#include <vix/arch/drivers/pic_8259.h>
#include <vix/arch/isr.h>
#include <vix/arch/paging.h>
#include <vix/debug.h>
#include <vix/drivers/net/generic_card.h>
#include <vix/kernel/irq.h>
#include <vix/macros.h>
#include <vix/mm/kheap.h>
#include <vix/net/stack/stack.h>
#include <vix/panic.h>
#include <vix/stdio.h>

#define RX_BUFFER_SIZE_IDX 1
#define RX_BUFFER_SIZE     (8192 << RX_BUFFER_SIZE_IDX)

static uint8_t bus;
static uint8_t device;
static uint8_t function;
static uint16_t io_base;
static uint8_t *bufferptr = nullptr;
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

static void irq_handler() {
    if ((((struct packetInfo *)(bufferptr + bufferoffset))->header & 0x1) == 0) {
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
    while ((inb(io_base + 0x37) & 0x1) == 0) {

        DEBUG_PRINTF("Buffer offset: %u\n", bufferoffset);

        // FIXME: i can imagine a bit of edge case where this would break when we have a buffer wrap!
        if (bufferoffset == RX_BUFFER_SIZE) {
            DEBUG_PRINTF("EDGE CASE");
        }
        if (bufferoffset > RX_BUFFER_SIZE) {
            DEBUG_PRINTF("EDGE CASE2");
        }
        if (bufferoffset == RX_BUFFER_SIZE) {
            bufferoffset = 0;
        }
        uint16_t packetSize = ((struct packetInfo *)(bufferptr + bufferoffset))->size;
        uint16_t packetSizeNoCRC = packetSize - 4;

        // these cases should be IMPOSSIBLE and if they do happen we're fucked
        if (packetSize <= 4 || packetSize > RX_BUFFER_SIZE) {
            KERNEL_PANIC("WTF");
        }

        void *packet = mm::kmalloc(packetSizeNoCRC);

        DEBUG_PRINTF("Buffer offset: %u Packet size: %u\n", bufferoffset, packetSize);

        // buffer wrap
        if ((bufferoffset + sizeof(struct packetInfo) + packetSize) > RX_BUFFER_SIZE) {
            size_t first_split = RX_BUFFER_SIZE - (bufferoffset + sizeof(struct packetInfo));

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

            memcpy(packet, ((uint8_t *)(bufferptr + bufferoffset)) + sizeof(struct packetInfo), n_read1);

            bufferoffset = 0;

            memcpy((uint8_t *)packet + first_split, (uint8_t *)(bufferptr + bufferoffset), n_read2);

            // set new buffer offset
            bufferoffset = ALIGN_UP(bufferoffset + (packetSize - first_split), 4);
            DEBUG_PRINTF("wbufferoffset %u\n", bufferoffset);
        } else {
            DEBUG_PRINTF("nbufferoffset1 %u\n", bufferoffset);
            memcpy(packet, ((uint8_t *)(bufferptr + bufferoffset)) + sizeof(struct packetInfo),
                   packetSizeNoCRC); // 4 CRC bytes
            // set new buffer offset
            bufferoffset = ALIGN_UP((bufferoffset + sizeof(packetInfo) + packetSize), 4);
            DEBUG_PRINTF("nbufferoffset2 %u\n", bufferoffset);
        }
        received.push_back({.ptr = packet, .size = packetSizeNoCRC});

        // set CAPR to offset of next expected packet
        // DEBUG_PRINTF("offset: %u\n", (uint32_t)bufferoffset);
        outw(io_base + 0x38, bufferoffset - 0x10);
    }

    outw(io_base + 0x3E, 0x5); // clear RX ok bit

    DEBUG_PRINTF("got %u packets\n", received.size());

    for (size_t i = 0; i < received.size(); i++) {
        networkstack.receive(received[i].ptr, received[i].size);
        mm::kfree(received[i].ptr);
    }
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
        DEBUG_PRINTF("rtl8139n't :c\n");
        return;
    } else {
        DEBUG_PRINTF("found rtl8139!\n");
    }

    if (drivers::pci::getBarType(bus, device, function, 0) != drivers::pci::bar_type::BAR_TYPE_IO) {
        DEBUG_PRINTF("tf is wrong with this rtl8139?\n");
        return;
    }

    io_base = drivers::pci::getBarIOAddress(bus, device, function, 0);
    if (!io_base) {
        DEBUG_PRINTF("couldn't get IO address for rtl8139\n");
        return;
    }

    if (drivers::pci::enableMastering(bus, device, function)) {
        DEBUG_PRINTF("enabled mastering for rtl8139!\n");
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

    DEBUG_PRINTF("rtl8139 mac: ");
    for (int i = 0; i < 5; i++) {
        DEBUG_PRINTF("%p:", (uint32_t)inb(io_base + 0x0 + i));
    }
    DEBUG_PRINTF("%p\n", (uint32_t)inb(io_base + 0x5));

    // receive buffer
    // rx buf len RX_BUFFER_SIZE + 16 byte
    bufferptr = (uint8_t *)mm::kmalloc_phys_contiguous(RX_BUFFER_SIZE + 16); // TODO: free
    outl(io_base + 0x30, (uintptr_t)paging::get_physaddr_unaligned(bufferptr));

    // outw(io_base + 0x3C, 0x0005); // Sets the TOK and ROK bits high

    outw(io_base + 0x3C, 0x1); // set ROK bit high

    // outw(io_base + 0x3C, 0xE1FF); // enable all possible interrupts

    // configure receive buffer
    outl(io_base + 0x44, 0xf | (0 << 7) | (RX_BUFFER_SIZE_IDX << 11)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

    // enable RX and TX
    outb(io_base + 0x37, 0x0C);
    irqline = drivers::pci::getInterruptLine(bus, device, function);
    DEBUG_PRINTF("rtl8139 irq: %u\n", (uint32_t)irqline);

    irq::register_irq_handler(irq_handler, irqline);

    DEBUG_PRINTF("rtl8139 init finished!\n");

    networkstack.init();
}

uint8_t drivers::net::rtl8139::get_mac_byte(int n) {
    if (n < 6) {
        return inb(io_base + 0x0 + n);
    }
    return 0;
}
