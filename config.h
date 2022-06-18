#define KERNEL_VIRT_ADDRESS 0xC0000000
#define KERNEL_PHYS_ADDRESS 0x00100000

// directory size: 4096 bytes(ends 0x1001000)
// tables size: 4194304 bytes
#define PAGE_DIRECTORY_OFFSET 0x1000000
#define PAGE_TABLES_OFFSET PAGE_DIRECTORY_OFFSET + (1024 * 4)

// location where ISR stores registers, size: 32 bytes 
//could be moved a bit
#define REGISTER_STORE_OFFSET 0x1402000 