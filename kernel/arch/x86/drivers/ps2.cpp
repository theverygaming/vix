#include <arch/cpubasics.h>
#include <arch/drivers/pic_8259.h>
#include <arch/drivers/ps2.h>
#include <arch/drivers/text80x25.h>
#include <arch/isr.h>
#include <cppstd/vector.h>
#include <drivers/ms_mouse.h>
#include <stdio.h>

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
    } else {
        printf("wrote 0x%p to mouse\n", (uint32_t)cmd);
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

static void kbdIntHandlerBase();
static void mouse_int_handler(isr::registers *) {
    // printf("mouse int\n");
    uint8_t status = inb(PS2_STATUS);
    if (!(status & 0x20)) { // is this not from port 2?
        kbdIntHandlerBase();
        return;
    }
    mouse_int_handler_base();
    drivers::pic::pic8259::eoi(drivers::pic::pic8259::irqToint(12));
}

void drivers::mouse::init() {
    isr::RegisterHandler(drivers::pic::pic8259::irqToint(12), mouse_int_handler);
    drivers::pic::pic8259::unmask_irq(12);

    // enable mouse interrupt
    ps2_write_command(0x20);
    uint8_t config = ps2_read_data();
    ps2_write_command(0x60);
    ps2_write_data(config | 0x22);

    ps2_mouse_send_command(0xF6); // set defaults
    ps2_mouse_send_command(0xF4); // enable data reporting
}

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

static bool shiftPressed = false;

static char readkbdchar() {
    char keycode = inb(0x60);
    if ((keycode == 0x2A) || (keycode == 0x36)) {
        return -2;
    }
    if (keycode > 0) {
        if (shiftPressed) {
            return kbd_US_sh[(unsigned char)keycode];
        }
        return kbd_US[(unsigned char)keycode];
    }
    return -1;
}

static void kbdIntHandlerBase() {
    char key = readkbdchar();
    if (key == '\b') {
        if (drivers::keyboard::bufferlocation > -1) {
            if (drivers::keyboard::bufferlocation < 100) {
                drivers::keyboard::buffer[drivers::keyboard::bufferlocation] = '\0';
            }
            drivers::keyboard::bufferlocation--;
            drivers::textmode::text80x25::delc();
            puts("\b \b");
        }
    } else if (key > 0) {
        printf("%c", key);
        drivers::keyboard::events.dispatch(&key);
        if (drivers::keyboard::bufferlocation < 100) {
            drivers::keyboard::buffer[++drivers::keyboard::bufferlocation] = key;
        } else {
            // printf("skill issue: keyboard buffer filled\n");
        }
    }
    if (key == -2) {
        shiftPressed = !shiftPressed;
    }
}

static void kbdIntHandler(isr::registers *) {
    uint8_t status = inb(PS2_STATUS);
    if (status & 0x20) { // is this from port 2?
        mouse_int_handler_base();
        return;
    }
    kbdIntHandlerBase();
    drivers::pic::pic8259::eoi(drivers::pic::pic8259::irqToint(1));
}

namespace drivers::keyboard {
    event_dispatcher<char> events;
    char buffer[100];
    int bufferlocation = -1;
}

void drivers::keyboard::init() {
    isr::RegisterHandler(drivers::pic::pic8259::irqToint(1), kbdIntHandler);
    drivers::pic::pic8259::unmask_irq(1);
}

void drivers::keyboard::poll() {
    char key = readkbdchar();
    if (key == '\b') {
        if (drivers::keyboard::bufferlocation > -1) {
            if (drivers::keyboard::bufferlocation < 100) {
                drivers::keyboard::buffer[drivers::keyboard::bufferlocation] = '\0';
            }
            drivers::keyboard::bufferlocation--;
            drivers::textmode::text80x25::delc();
            puts("\b \b");
        }
    } else if (key > 0) {
        printf("%c", key);
        if (drivers::keyboard::bufferlocation < 100) {
            drivers::keyboard::buffer[++drivers::keyboard::bufferlocation] = key;
        } else {
            // printf("skill issue: keyboard buffer filled\n");
        }
    }
    if (key == -2) {
        shiftPressed = !shiftPressed;
    }
    while (readkbdchar() == key) {}
}
