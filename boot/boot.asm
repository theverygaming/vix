; Will write this myself later when i actually understand all this, for now copypasted from https://github.com/AlexandreRouma/PenutOS/blob/master/boot/bootsect.asm

%define LD_ADDRESS 0x100 ; Address of where to load the kernel
%define KRN_SIZE   54      ; Kernel size in sectors
%define FST_SECTOR 2      ; Sector of the kernel

[bits 16]
[org 0x0]

mov ax, 0x07C0 ; Initialization
mov ds, ax
mov es, ax

mov ax, 0x8000 ; Loading stack
mov ss, ax
mov sp, 0xf000

mov [boot_device], dl ; Get bootdisk number

mov ah, 0x00 ; Set video mode to 0x03 (80 - 25)
mov al, 0x03
int 0x10

xor ax, ax
int 0x13

push es

mov ax, LD_ADDRESS
mov es, ax
mov bx, 0
mov ah, 0x02 ; Load the kernel
mov al, KRN_SIZE
mov ch, 0x00
mov cl, FST_SECTOR
mov dh, 0x00
mov dl, [boot_device]
int 0x13

pop es

mov ax, gdtend    ; calcule la limite de GDT
mov bx, gdt
sub ax, bx
mov word [gdtptr], ax
xor eax, eax      ; calcule l'adresse lineaire de GDT
xor ebx, ebx
mov ax, ds
mov ecx, eax
shl ecx, 4
mov bx, gdt
add ecx, ebx
mov dword [gdtptr+2], ecx

cli
lgdt [gdtptr]    ; charge la gdt
mov eax, cr0
or  ax, 1
mov cr0, eax 

jmp next
next:
mov ax, 0x10
mov ds, ax
mov fs, ax
mov gs, ax
mov es, ax
mov ss, ax
mov esp, 0x9F000

jmp dword 0x8:0x1000


; Variables
boot_device db 0

gdt:
    db 0, 0, 0, 0, 0, 0, 0, 0
gdt_cs:
    db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10011011b, 11011111b, 0x0
gdt_ds:
    db 0xFF, 0xFF, 0x0, 0x0, 0x0, 10010011b, 11011111b, 0x0
gdtend:

gdtptr:
    dw 0

times 510-($-$$) db 144 ; NOP until 510 bytes
dw 0xAA55 ; Bootloader signature