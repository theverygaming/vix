#include <arch/x86/drivers/net/rtl8139.h>
#include <arch/x86/cpubasics.h>
#include <arch/x86/drivers/pci.h>
#include <arch/x86/isr.h>
#include <arch/x86/paging.h>
#include <memory_alloc/memalloc.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t bus;
static uint8_t device;
static uint8_t function;
static uint16_t io_base;
void *bufferptr = nullptr;

static void irq_handler(isr::registers *gaming) {
    printf("got rtl8139 IRQ\n");
    outw(io_base + 0x3E, 0x5); // Interrupt Status - Clears the Rx OK bit, acknowledging a packet has been received, and is now in rx_buffer
    // outb(0x20, 0x20);
}

static uint8_t reg_counter = 0;
void drivers::net::rtl8139::sendPacket(void *data, uint32_t len) {
    /* https://wiki.osdev.org/RTL8139
     * The RTL8139 NIC uses a round robin style for transmitting packets. It has four transmit buffer (a.k.a. transmit start) registers, and four transmit status/command registers. The transmit start
     * registers are each 32 bits long, and are in I/O offsets 0x20, 0x24, 0x28 and 0x2C. The transmit status/command registers are also each 32 bits long and are in I/O offsets 0x10, 0x14, 0x18 and
     * 0x1C. Each pair of transmit start and status registers work together (i.e. registers 0x20 and 0x10 work together, 0x24 and 0x14 work together, etc.)
     */
    if(reg_counter > 3) { reg_counter = 0; }
    outl(io_base + 0x20 + (reg_counter * 4), (uint32_t)paging::get_physaddr_unaligned(data)); // transmit
    outl(io_base + 0x10 + (reg_counter * 4), len); // status/command
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

    // receive buffer
    bufferptr = memalloc::single::kmalloc(8192 + 16); // TODO: possibly free this later
    outl(io_base + 0x30, (uint32_t)paging::get_physaddr_unaligned(bufferptr));

    // TODO: make this not broken
    isr::RegisterHandler(34, irq_handler);
    outw(io_base + 0x3C, 0x0005); // Sets the TOK and ROK bits high

    // configure receive buffer
    outl(io_base + 0x44, 0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

    // enable RX and TX
    outb(io_base + 0x37, 0x0C);
    drivers::pci::writeInterruptLine(bus, device, function, 2);

    // send packet
    outl(io_base + 0x20, (uint32_t)0xFFFF);
    outl(io_base + 0x10, 100);

    printf("rtl8139 init finished!\n");
}
