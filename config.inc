%define KERNEL_VIRT_ADDRESS 0xC0000000
%define KERNEL_PHYS_ADDRESS 0x00100000

; location where ISR stores registers, size: 32 bytes 
; could be moved a bit
%define REGISTER_STORE_OFFSET 0x1402000