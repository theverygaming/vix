%define KERNEL_VIRT_ADDRESS 0xC0000000
%define KERNEL_PHYS_ADDRESS 0x1F0000

; location where ISR stores registers, size: 32 bytes 
; could be moved a bit
%define REGISTER_STORE_OFFSET 0x502000

%define KERNEL_ISR_STACK_POINTER_OFFSET 0xF49000