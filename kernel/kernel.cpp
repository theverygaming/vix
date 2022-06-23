void kernelstart();

//this will be loaded first at boot
void _start(void)
{
	kernelstart();
}

#include "cpubasics.h"
#include "stdlib.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "stdio.h"
#include "paging.h"
#include "memorymap.h"
#include "../config.h"


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

void handler(isr::Registers* gaming) {
	signed char keycode = inb(0x60);
	if(keycode > 0) {
		printf("%c", kbd_US[keycode]);
	}
	outb(0x20, 0x20);
}

void kernelstart()
{
  //uint32_t sp_adr = KERNEL_VIRT_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET;
  //asm("mov %0, %%esp" : "=r"(sp_adr) :);
  clrscr();
  //paging::clearPageTables((void*)0x0, 6);
  //register uint32_t esp asm("esp");
	printf("hewwo\n");
  register uint32_t esp asm("esp");
  printf("esp: %p\n", esp);
  cpubasics::cpuinit();
  isr::RegisterHandler(33, handler);
  //paging::clearPageTables((void*)0x0, 100000);
  for(;;);
  //for(;;);
	//cpubasics::cpuinit();
  //memorymap::initMemoryMap((void*)0x7C00 + 0x7000, (void*)0x7C00 + (0x7004));
	//__asm("int $33");
	//printf("registering handler\n");
	//isr::RegisterHandler(33, handler);
  //cpubasics::sleep(100);
	//__asm("int $33");
	//isr::DeregisterHandler(33);
	printf("ayy\n");
  //cpubasics::sleep(100);
  printf("initializing paging\n");
  //paging::initpaging();
  //printf("initialized paging\n");
  //__asm("int $35");
  int counter = 0;
    while(1) {
      //cpubasics::sleep(10);
      *((unsigned char *)((KERNEL_VIRT_ADDRESS + VIDMEM_OFFSET) + 2 * 70 + 160 * 0)) = counter / 20;
      //printf("HAI ");
      //printf("%llu ", *balls);
      counter++;
      if(counter == 1000) { counter = 0; }
    }
}