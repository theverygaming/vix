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
#include "memalloc.h"

// very important arrays definitely
uint8_t zerotwo[18][13] = {
    { 15,15,15,15,13,13,13,13,13,15,15,15,15},
    { 15,15,15,13,13,13,13,13,13,13,15,15,15},
    { 15,15,13,13,4,13,13,13,4,13,13,15,15},
    { 15,15,13,7,4,7,7,7,4,7,13,15,15},
    { 15,15,13,13,13,13,13,13,13,13,13,15,15},
    { 15,15,13,13,14,14,14,14,14,13,13,15,15},
    { 15,15,13,13,14,2,14,2,14,13,13,15,15},
    { 15,15,13,13,14,14,14,14,14,13,13,15,15},
    { 15,15,13,13,0,15,14,15,0,13,13,15,15},
    { 15,15,7,7,15,15,6,15,15,7,7,15,15},
    { 15,15,4,8,4,15,6,15,4,8,4,15,15},
    { 15,0,4,8,4,4,8,4,4,8,4,0,15},
    { 0,14,14,0,4,4,4,4,4,0,14,14,0},
    { 0,14,14,0,4,4,8,4,4,0,14,14,0},
    { 15,0,0,8,8,7,4,7,8,8,0,0,15},
    { 15,15,15,0,4,4,0,4,4,0,15,15,15},
    { 15,15,15,0,4,4,0,4,4,0,15,15,15},
    { 15,15,15,15,0,0,15,0,0,15,15,15,15},
};

uint8_t franxxlogo[9][18] = {
    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,4,0},
    { 0,0,0,0,0,0,0,15,0,15,0,0,0,4,0,4,0,0},
    { 0,0,1,0,1,0,1,0,15,0,4,0,4,0,4,0,0,0},
    { 0,0,0,1,0,1,0,1,0,4,0,4,0,4,0,0,0,0},
    { 0,0,0,0,1,0,1,0,0,0,4,0,4,0,0,0,0,0},
    { 0,0,0,1,0,1,0,1,0,4,0,4,0,4,0,0,0,0},
    { 0,0,1,0,1,0,1,0,15,0,4,0,4,0,4,0,0,0},
    { 0,1,0,1,0,0,0,15,0,15,0,0,0,4,0,4,0,0},
    { 1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

void kernelstart()
{
  //uint32_t sp_adr = KERNEL_VIRT_ADDRESS + KERNEL_START_STACK_POINTER_OFFSET;
  //asm("mov %0, %%esp" : "=r"(sp_adr) :);
  //clrscr();
  //paging::clearPageTables((void*)0x0, 6);
  //register uint32_t esp asm("esp");
	printf("\n\n\n\n");
  printf("hewwo\n");
  register uint32_t esp asm("esp");
  printf("esp: %p\n", esp);
  memorymap::initMemoryMap((void*)0x7C00 + 0x7000, (void*)0x7C00 + (0x7004));
  paging::clearPageTables((void*)0x0, 10000);
  memalloc::page::phys_init(memorymap::map_entries, memorymap::map_entrycount);
  cpubasics::cpuinit();
  drivers::keyboard::init();
  isr::RegisterHandler(0x80, syscall::syscallHandler);
  // program is loaded at 0x4C4C000
  for(uint32_t i = 0; i < 0xFFFFFFF; i++) {}
  elf::load_program((void*)0x4C4C000);
  
  for(int i = 0; i < 18; i++) {
      for(int j = 0; j < 13; j++) {
          putcolor(j+67, i, zerotwo[i][j] << 4 | 0);
      }
  }
  for(int i = 0; i < 9; i++) {
      for(int j = 0; j < 18; j++) {
          putcolor(j+49, i, franxxlogo[i][j] << 4 | 7);
      }
  }
  
  multitasking::process_pagerange prange[PROCESS_MAX_PAGE_RANGES];
  multitasking::createPageRange(prange);
  for(int i = 0; i < PROCESS_MAX_PAGE_RANGES; i++) {
    if(prange[i].pages != 0) {
      printf("created prange, pb: 0x%p vb: 0x%p pg: %u, i: %d\n", prange[i].phys_base, prange[i].virt_base, prange[i].pages, i);
    }
  }
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