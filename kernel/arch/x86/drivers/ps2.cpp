#include <arch/common/cpu.h>
#include <arch/cpubasics.h>
#include <arch/drivers/pic_8259.h>
#include <arch/drivers/ps2.h>
#include <arch/drivers/text80x25.h>
#include <arch/isr.h>
#include <drivers/ms_mouse.h>
#include <keyboard.h>
#include <stdio.h>
#include <vector>

#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_COMMAND 0x64

static bool ps2_wait_write() {
    for (uint32_t i = 0; i < 500000; i++) {
        if ((inb(PS2_STATUS) & 0x2) == 0x0) {
            return true;
        }
    }
    printf("wait write timeout\n");
    return false;
}

static bool ps2_wait_read() {
    for (uint32_t i = 0; i < 500000; i++) {
        if ((inb(PS2_STATUS) & 0x1) == 0x0) {
            return true;
        }
    }
    printf("wait read timeout\n");
    return false;
}

static void ps2_write_command(uint8_t value) {
    if (ps2_wait_write()) {
        outb(PS2_COMMAND, value);
    }
}

static void ps2_write_data(uint8_t value) {
    if (ps2_wait_write()) {
        outb(PS2_DATA, value);
    }
}

static uint8_t ps2_read_data() {
    if (ps2_wait_read()) {
        return inb(PS2_DATA);
    }
    return 0;
}

static void ps2_mouse_send_command(uint8_t cmd) {
    outb(PS2_COMMAND, 0xD4);
    ps2_write_data(cmd);
    uint8_t status = ps2_read_data();
    if (status != 0xFA) {
        printf("ps2 mouse read failure (0x%p)\n", (uintptr_t)status);
    }
}

static uint8_t mouse_packets[4];
static uint8_t mouse_counter = 1; // have to start at 1, unknown why

static void mouse_int_handler_base() {
    mouse_packets[mouse_counter++] = inb(PS2_DATA);
    if (mouse_counter > 2) {
        mouse_counter = 0;
        drivers::ms_mouse::rx_packet(mouse_packets, 3);
    }
}

static void kbd_int_handler_base();
static void mouse_int_handler(struct arch::full_ctx *) {
    // printf("mouse int\n");
    uint8_t status = inb(PS2_STATUS);
    if (!(status & 0x20)) { // is this not from port 2?
        kbd_int_handler_base();
        return;
    }
    mouse_int_handler_base();
    drivers::pic::pic8259::eoi(drivers::pic::pic8259::irqToint(12));
}

void drivers::mouse::init() {
    isr::RegisterHandler(drivers::pic::pic8259::irqToint(12), mouse_int_handler);
    drivers::pic::pic8259::unmask_irq(12);

    ps2_write_command(0x20);
    uint8_t config = ps2_read_data();
    // enable mouse interrupt
    if ((config & (1 << 5)) != 0) {
        ps2_write_command(0x60);
        ps2_write_data(config | 0x22);

        ps2_mouse_send_command(0xF6); // set defaults
        ps2_mouse_send_command(0xF4); // enable data reporting
    }
}

// clang-format off
char kbd_US[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-',  '=',  '\b', '\t',                                                    /* <-- Tab */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']', '\n', 0,                                                                   /* <-- control key */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,    '\\', 'z',  'x',  'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, /* Alt */
    ' ',                                                                                                                                    /* Space bar */
    0,                                                                                                                                      /* Caps lock */
    0,                                                                                                                                      /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,   0,                                                                                              /* < ... F10 */
    0,                                                                                                                                      /* 69 - Num lock*/
    0,                                                                                                                                      /* Scroll Lock */
    0,                                                                                                                                      /* Home key */
    0,                                                                                                                                      /* Up Arrow */
    0,                                                                                                                                      /* Page Up */
    '-', 0,                                                                                                                                 /* Left Arrow */
    0,   0,                                                                                                                                 /* Right Arrow */
    '+', 0,                                                                                                                                 /* 79 - End key*/
    0,                                                                                                                                      /* Down Arrow */
    0,                                                                                                                                      /* Page Down */
    0,                                                                                                                                      /* Insert Key */
    0,                                                                                                                                      /* Delete Key */
    0,   0,   0,   0,                                                                                                                       /* F11 Key */
    0,                                                                                                                                      /* F12 Key */
    0,                                                                                                                                      /* All other keys are undefined */
};

char kbd_US_sh[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',  '+', '\b', '\t',                                                    /* <-- Tab */
    'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,                                                                  /* <-- control key */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '?', 0,   0,    '|', 'Z',  'X',  'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, /* Alt */
    ' ',                                                                                                                                  /* Space bar */
    0,                                                                                                                                    /* Caps lock */
    0,                                                                                                                                    /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,   0,                                                                                            /* < ... F10 */
    0,                                                                                                                                    /* 69 - Num lock*/
    0,                                                                                                                                    /* Scroll Lock */
    0,                                                                                                                                    /* Home key */
    0,                                                                                                                                    /* Up Arrow */
    0,                                                                                                                                    /* Page Up */
    '-', 0,                                                                                                                               /* Left Arrow */
    0,   0,                                                                                                                               /* Right Arrow */
    '+', 0,                                                                                                                               /* 79 - End key*/
    0,                                                                                                                                    /* Down Arrow */
    0,                                                                                                                                    /* Page Down */
    0,                                                                                                                                    /* Insert Key */
    0,                                                                                                                                    /* Delete Key */
    0,   0,   0,   0,                                                                                                                     /* F11 Key */
    0,                                                                                                                                    /* F12 Key */
    0,                                                                                                                                    /* All other keys are undefined */
};
// clang-format on

// TODO: in the future base on Minix code. https://github.com/Stichting-MINIX-Research-Foundation/minix/blob/master/minix/drivers/hid/pckbd/pckbd.c

static void kbd_int_handler_base() {
    static bool extended = false;
    static bool shift = false;

    uint8_t sc = inb(0x60);

    if (sc == 0xE0) {
        extended = true;
        return;
    }

    if (sc & 0x80) { // key released
        sc &= 0x7F;
        drivers::keyboard::raw_release_events.dispatch(sc);
        if (sc == KEY_LEFTSHIFT || sc == KEY_RIGHTSHIFT) {
            shift = false;
        }
        return;
    }

    if (extended) {
        extended = false;
        return;
    }

    drivers::keyboard::raw_press_events.dispatch(sc);

    if (sc == KEY_LEFTSHIFT || sc == KEY_RIGHTSHIFT) { // hacky, works fine as long as you don't use two shift keys at once lol
        shift = true;
        return;
    }

    char *kbmap = kbd_US;
    if (shift) {
        kbmap = kbd_US_sh;
    }

    if (kbmap[sc] == 0 || kbmap[sc] == 27) {
        return;
    }

    putc(kbmap[sc]);

    drivers::keyboard::events.dispatch(kbmap[sc]);
}

static void ps2_int(struct arch::full_ctx *) {
    uint8_t status = inb(PS2_STATUS);
    if (status & 0x20) { // is this from port 2?
        mouse_int_handler_base();
        return;
    }
    kbd_int_handler_base();
    drivers::pic::pic8259::eoi(drivers::pic::pic8259::irqToint(1));
}

namespace drivers::keyboard {
    event_dispatcher<char> events;
    event_dispatcher<uint8_t> raw_press_events;
    event_dispatcher<uint8_t> raw_release_events;
}

void drivers::keyboard::init() {
    isr::RegisterHandler(drivers::pic::pic8259::irqToint(1), ps2_int);
    drivers::pic::pic8259::unmask_irq(1);
}
