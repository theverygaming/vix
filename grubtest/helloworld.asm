[BITS 32]
org 0
jmp start
; https://intermezzos.github.io/book/first-edition/multiboot-headers.html
align 8
header_start:
    dd 0xe85250d6                ; magic number
    dd 0                         ; protected mode code
    dd header_end - header_start ; header length

    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

start:
mov al, 69
out 0xE9, al
jmp start