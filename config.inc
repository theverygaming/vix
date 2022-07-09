%define KERNEL_VIRT_ADDRESS 0xC0000000
%define KERNEL_PHYS_ADDRESS 0x01CA5000

; location where ISR stores registers, size: 32 bytes 
; could be moved a bit
%define REGISTER_STORE_OFFSET 0x1401FE0

%define KERNEL_ISR_STACK_POINTER_OFFSET 0x211D000