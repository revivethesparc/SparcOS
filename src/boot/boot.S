.section .text32, "ax"
.code32

.align 4
.long 0x1BADB002
.long 0x03
.long -(0x1BADB002 + 0x03)

.align 16
gdt64:
.quad 0x0000000000000000
.quad 0x00CF9A000000FFFF
.quad 0x00CF92000000FFFF
.quad 0x00AF9A000000FFFF
gdt64_ptr:
.word 31
.long gdt64

.global _start
_start:
    cli
    mov $pml4, %edi
    xor %eax, %eax
    mov $(4096*3), %ecx
    rep stosb

    mov $pdpt, %eax
    or $0x03, %eax
    mov %eax, pml4

    mov $pd, %eax
    or $0x03, %eax
    mov %eax, pdpt

    mov $0x83, %eax
    mov %eax, pd

    mov $pml4, %eax
    mov %eax, %cr3

    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4

    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr

    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0

    lgdt gdt64_ptr
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x18, $_start64

.code64
.global _start64
_start64:
    mov $stack_top, %rsp
    call kernel_main
    cli
1:  hlt
    jmp 1b

.macro isr_noerr n
.global isr\n
.type isr\n, @function
isr\n:
    push $0
    push $\n
    jmp isr_common_stub
.endm

.macro isr_err n
.global isr\n
.type isr\n, @function
isr\n:
    push $\n
    jmp isr_common_stub
.endm

.macro irq_stub n
.global irq\n
.type irq\n, @function
irq\n:
    push $0
    push $(32 + \n)
    jmp irq_common_stub
.endm

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err   8
isr_noerr 9
isr_err   10
isr_err   11
isr_err   12
isr_err   13
isr_err   14
isr_noerr 15
isr_noerr 16
isr_err   17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31

irq_stub 0
irq_stub 1
irq_stub 2
irq_stub 3
irq_stub 4
irq_stub 5
irq_stub 6
irq_stub 7
irq_stub 8
irq_stub 9
irq_stub 10
irq_stub 11
irq_stub 12
irq_stub 13
irq_stub 14
irq_stub 15

.section .rodata
.global isr_stubs
isr_stubs:
.irp i,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
.quad isr\i
.endr

.global irq_stubs
irq_stubs:
.irp i,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
.quad irq\i
.endr

.text
isr_common_stub:
    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rdi
    push %rsi
    push %rbp
    push %rdx
    push %rcx
    push %rbx
    push %rax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    cld
    push %rsp
    call isr_handler
    add $8, %rsp
    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx
    pop %rbp
    pop %rsi
    pop %rdi
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15
    add $16, %rsp
    iretq

irq_common_stub:
    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rdi
    push %rsi
    push %rbp
    push %rdx
    push %rcx
    push %rbx
    push %rax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    cld
    push %rsp
    call irq_handler
    add $8, %rsp
    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx
    pop %rbp
    pop %rsi
    pop %rdi
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15
    add $16, %rsp
    iretq

.section .bss
.align 4096
pml4:   .skip 4096
pdpt:   .skip 4096
pd:     .skip 4096
stack_bottom:
.skip 16384
stack_top:
