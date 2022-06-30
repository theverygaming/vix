#define KERNEL_VIRT_ADDRESS 0xC0000000
#define KERNEL_PHYS_ADDRESS 0x01CA5000

// directory size: 4096 bytes(ends 0x1001000)
// tables size: 4194304 bytes
#define PAGE_DIRECTORY_OFFSET 0x1000000
#define PAGE_TABLES_OFFSET PAGE_DIRECTORY_OFFSET + (1024 * 4)

// location where ISR stores registers, size: 32 bytes 
//could be moved a bit
#define REGISTER_STORE_OFFSET 0x1401FE0

// 80 * 25 * 2 = 4000 bytes, will be 4096 bytes bc it's paged
#define VIDMEM_OFFSET 0x1402000

#define ISR_HANDLER_OFFSET 0x1403000 // TODO: add size

#define KERNEL_START_STACK_POINTER_OFFSET 0x2109000 // stack grows downwards

#define GDT_OFFSET 0x2109000