#include "serial.h"

#define SERIAL_PORT 0x3f8 // 0x3f8 is COM1

bool serial_enabled = false;

void drivers::serial::init() {
    // straight from osdev wiki
    outb(SERIAL_PORT + 1, 0x00); // Disable all interrupts
    outb(SERIAL_PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(SERIAL_PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(SERIAL_PORT + 1, 0x00); //                  (hi byte)
    outb(SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(SERIAL_PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(SERIAL_PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(SERIAL_PORT + 0) != 0xAE)
    {
        serial_enabled = false;
        return;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(SERIAL_PORT + 4, 0x0F);
    serial_enabled = true;
}

static int is_tx_empty() {
    return inb(SERIAL_PORT + 5) & 0x20;
}

void drivers::serial::putc(char c) {
    if(!serial_enabled) { return; }
    while(is_tx_empty() == 0);
    outb(SERIAL_PORT, c);
    if(c == '\n') {
        outb(SERIAL_PORT, '\r');
    }
}

char drivers::serial::getc() {
    if(!serial_enabled) { return '\0'; }
    while(!(inb(SERIAL_PORT + 5) & 1));
    return inb(SERIAL_PORT);
}