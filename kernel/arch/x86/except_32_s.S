.section .text

.macro defexcept_entry_noerr num
.global except_entry\num
except_entry\num:
push $0 // error code
push $\num // interrupt number
jmp except_entry
.endm

.macro defexcept_entry_err num
.global except_entry\num
except_entry\num:
push $\num // interrupt number
jmp except_entry
.endm

defexcept_entry_noerr 0
defexcept_entry_noerr 1
defexcept_entry_noerr 2
defexcept_entry_noerr 3
defexcept_entry_noerr 4
defexcept_entry_noerr 5
defexcept_entry_noerr 6
defexcept_entry_noerr 7
defexcept_entry_err 8
defexcept_entry_noerr 9
defexcept_entry_err 10
defexcept_entry_err 11
defexcept_entry_err 12
defexcept_entry_err 13
defexcept_entry_err 14
defexcept_entry_noerr 15
defexcept_entry_noerr 16
defexcept_entry_err 17
defexcept_entry_noerr 18
defexcept_entry_noerr 19
defexcept_entry_noerr 20
defexcept_entry_err 21
defexcept_entry_noerr 22
defexcept_entry_noerr 23
defexcept_entry_noerr 24
defexcept_entry_noerr 25
defexcept_entry_noerr 26
defexcept_entry_noerr 27
defexcept_entry_noerr 28
defexcept_entry_err 29
defexcept_entry_err 30
defexcept_entry_noerr 31

except_entry:
    push %ds
    push %es
    push %fs
    push %gs

    push %eax
    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    
    push %ebp

    push %esp
    mov $(2 * 8), %ax // use kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    call handle_x86_except
    add $4, %esp

    pop %ebp

    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax

    pop %gs
    pop %fs
    pop %es
    pop %ds

    add $8, %esp // error code + interrupt number

    iretl
