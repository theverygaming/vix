#pragma once

/* required */

/* custom */

#define KERNEL_VIRT_ADDRESS 0xC0000000
#define KERNEL_PHYS_ADDRESS 0x1EA000

#define KERNEL_CODE_SIZE 0xF0000

// directory size: 4096 bytes(ends 0x1001000)
// tables size: (1024 * 1024 * 4) bytes
#define PAGE_DIRECTORY_OFFSET 0xF0000
#define PAGE_TABLES_OFFSET    (PAGE_DIRECTORY_OFFSET + (1024 * 4))

// location where ISR stores registers, size: 32 bytes
// could be moved a bit
// #define REGISTER_STORE_OFFSET 0x602000

#define GDT_OFFSET 0x4F1000

// 80 * 25 * 2 = 4000 bytes, will be 4096 bytes bc it's paged
#define VIDMEM_OFFSET 0x4F2000

#define KERNEL_START_STACK_POINTER_OFFSET 0x501000 // stack grows downwards
#define KERNEL_ISR_STACK_POINTER_OFFSET   0x50B000

#define KERNEL_FREE_AREA_BEGIN_OFFSET 0x50B000
#define KERNEL_MEMORY_END_OFFSET      0x9C1000
