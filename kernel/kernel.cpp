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
#include "elf.h"
#include "multitasking.h"
#include "syscall.h"
#include "drivers/keyboard.h"

void kernelstart()
{
  paging::clearPageTables((void*)0x0, 10000);
  //uint32_t sp_adr = KERNEL_VIRT_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET;
  //asm("mov %0, %%esp" : "=r"(sp_adr) :);
  //clrscr();
  //paging::clearPageTables((void*)0x0, 6);
  //register uint32_t esp asm("esp");
	printf("\n\n\n\n");
  printf("hewwo\n");
  register uint32_t esp asm("esp");
  printf("esp: %p\n", esp);
  cpubasics::cpuinit();
  drivers::keyboard::init();
  isr::RegisterHandler(0x80, syscall::syscallHandler);
  // program is loaded at 0x4C4C000
  for(uint32_t i = 0; i < 0xFFFFFFF; i++) {}
  elf::load_program((void*)0x4C4C000);
  //for(;;);
	//cpubasics::cpuinit();
  //memorymap::initMemoryMap((void*)0x7C00 + 0x7000, (void*)0x7C00 + (0x7004));
	//__asm("int $33");
	//printf("registering handler\n");
	//isr::RegisterHandler(33, handler);
  //cpubasics::sleep(100);
	//__asm("int $33");
	//isr::DeregisterHandler(33);
	//printf("ayy\n");
  //cpubasics::sleep(100);
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