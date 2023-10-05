// extern "C" void sched_switch(struct arch::ctx **old, struct arch::ctx *_new);
.section .text
.global sched_switch
sched_switch:
    movl 4(%esp), %eax // old
    movl 8(%esp), %ecx // new

    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %ebx

    movl %esp, (%eax)
    movl %ecx, %esp

    popl %ebx
    popl %esi
    popl %edi
    popl %ebp
    ret
