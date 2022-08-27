[BITS 32]
org 0xFFF000
; https://intermezzos.github.io/book/first-edition/multiboot-headers.html
align 8
header_start:
    dd 0xe85250d6                ; magic number
    dd 0                         ; architecture -> 0 = 32-bit protected mode i386
    dd header_end - header_start ; header length

    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    align 8

    dw 2 ; type = 2(address tag)
    dw 0 ; flags
    dd 24 ; size
    dd header_start ; header_addr
    dd -1 ; load_addr -> -1 means must be loaded from beginning
    dd 0 ; load_end_addr -> 0 means .data and .text is whole image
    dd 0 ; bss_end_addr

    align 8

    dw 3 ; type = 3(entry address tag)
    dw 0 ; flags
    dd 12 ; size
    dd start ; entry point

    align 8

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

start:
mov al, 69
out 0xE9, al
jmp start