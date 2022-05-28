; Will write this myself later when i actually understand all this, for now copypasted from https://github.com/AlexandreRouma/PenutOS/blob/master/boot/bootsect.asm

%define LD_ADDRESS 0x100 ; Address of where to load the kernel
%define KRN_SIZE   54      ; Kernel size in sectors
%define FST_SECTOR 2      ; Sector of the kernel

[bits 16]
[org 0x0]
jmp init

mmap_ent equ 0x8000             ; the number of entries will be stored at 0x8000
do_e820:
    mov di, 0x8004          ; Set di to 0x8004. Otherwise this code will get stuck in `int 0x15` after some entries are fetched 
	xor ebx, ebx		; ebx must be 0 to start
	xor bp, bp		; keep an entry count in bp
	mov edx, 0x0534D4150	; Place "SMAP" into edx
	mov eax, 0xe820
	mov [0x8004 + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes
	int 0x15
	jc short .failed	; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150	; Some BIOSes apparently trash this register?
	cmp eax, edx		; on success, eax must have been reset to "SMAP"
	jne short .failed
	test ebx, ebx		; ebx = 0 implies list is only 1 entry long (worthless)
	je short .failed
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820		; eax, ecx get trashed on every int 0x15 call
	mov [0x8004 + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes again
	int 0x15
	jc short .e820f		; carry set means "end of list already reached"
	mov edx, 0x0534D4150	; repair potentially trashed register
.jmpin:
	jcxz .skipent		; skip any 0 length entries
	cmp cl, 20		; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [0x8004 + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skipent
.notext:
	mov ecx, [0x8004 + 8]	; get lower uint32_t of memory region length
	or ecx, [0x8004 + 12]	; "or" it with upper uint32_t to test for zero
	jz .skipent		; if length uint64_t is 0, skip entry
	inc bp			; got a good entry: ++count, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx		; if ebx resets to 0, list is complete
	jne short .e820lp
.e820f:
	mov [mmap_ent], bp	; store the entry count
	clc			; there is "jc" on end of list to this point, so the carry must be cleared
	ret
.failed:
    jmp .failed ; we do a little infinite looping
	stc			; "function unsupported" error exit
	ret



init:
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

pusha
call do_e820
popa


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