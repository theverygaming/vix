#include <arch/x86/cpubasics.h>
#include <arch/x86/drivers/keyboard.h>
#include <arch/x86/drivers/text80x25.h>
#include <arch/x86/isr.h>
#include <stdio.h>

namespace drivers::keyboard {
    char buffer[100];
    int bufferlocation = -1;
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

static void kbdIntHandler(isr::registers *) {
    char key = readkbdchar();
    if (key == '\b') {
        if (drivers::keyboard::bufferlocation > -1) {
            if (drivers::keyboard::bufferlocation < 100) {
                drivers::keyboard::buffer[drivers::keyboard::bufferlocation] = '\0';
            }
            drivers::keyboard::bufferlocation--;
            drivers::textmode::text80x25::delc();
        }
    } else if (key > 0) {
        printf("%c", key);
        if (drivers::keyboard::bufferlocation < 100) {
            drivers::keyboard::buffer[++drivers::keyboard::bufferlocation] = key;
        } else {
            printf("skill issue: keyboard buffer filled\n");
        }
    }
    if (key == -2) {
        shiftPressed = !shiftPressed;
    }
    outb(0x20, 0x20);
}

void drivers::keyboard::init() {
    isr::RegisterHandler(33, kbdIntHandler);
}

void drivers::keyboard::manualRead() {
    char key = readkbdchar();
    if (key == '\b') {
        if (drivers::keyboard::bufferlocation > -1) {
            if (drivers::keyboard::bufferlocation < 100) {
                drivers::keyboard::buffer[drivers::keyboard::bufferlocation] = '\0';
            }
            drivers::keyboard::bufferlocation--;
            drivers::textmode::text80x25::delc();
        }
    } else if (key > 0) {
        printf("%c", key);
        if (drivers::keyboard::bufferlocation < 100) {
            drivers::keyboard::buffer[++drivers::keyboard::bufferlocation] = key;
        } else {
            printf("skill issue: keyboard buffer filled\n");
        }
    }
    if (key == -2) {
        shiftPressed = !shiftPressed;
    }
    while (readkbdchar() == key)
        ;
}
