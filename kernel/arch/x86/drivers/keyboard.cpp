#include "keyboard.h"
#include <arch/x86/isr.h>
#include <stdio.h>
#include <arch/x86/cpubasics.h>

namespace drivers::keyboard {
    char buffer[100];
    int bufferlocation = -1;
}

char kbd_US [128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   
  '\t', /* <-- Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     
    0, /* <-- control key */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

char readkbdchar() {
    char keycode = inb(0x60);
    if(keycode > 0) {
      return kbd_US[keycode];
    }
    return -1;
}

void kbdIntHandler(isr::Registers* gaming) {
	char key = readkbdchar();
	if(key > 0) {
		printf("%c", key);
    if(drivers::keyboard::bufferlocation < 100) {
      drivers::keyboard::buffer[++drivers::keyboard::bufferlocation] = key;
    }
    else{
      printf("keyboard buffer filled\n");
    }
	}
	outb(0x20, 0x20);
}

void drivers::keyboard::init() {
    isr::RegisterHandler(33, kbdIntHandler);
}

void drivers::keyboard::manualRead() {
    char key = readkbdchar();
    kbdIntHandler(nullptr);
    while(readkbdchar() == key);
}