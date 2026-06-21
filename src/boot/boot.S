/**
 * @file    boot.S
 * @brief   x86_64 kernel entry point — Multiboot header, page tables, and ISR stubs
 * @details This file contains the GRUB-compatible Multiboot header, the 32-bit
 *          entry point that transitions the CPU to long mode (x86_64), and all
 *          assembly-level interrupt service routine (ISR) / IRQ stubs. The
 *          stubs preserve registers, dispatch to C handlers, and return via
 *          iretq.
 * @info    The Multiboot header MUST reside in .text32 (a PT_LOAD segment)
 *          within the first 8192 bytes of the final binary, or GRUB will not
 *          find it.
 * @note    Page tables are statically allocated in .bss and identity-map the
 *          first 2 MB using a single 2 MiB huge page.
 * @author  OS Framework Team
 * @date    2026
 */

/* ========================================================================== */
/*  Section: .text32 — 32-bit entry & Multiboot header                        */
/* ========================================================================== */
.section .text32, "ax"
.code32

/* Multiboot header — must appear within first 8192 bytes */
.align 4
.long 0x1BADB002              /* @brief  Magic: Multiboot signature         */
.long 0x03                    /* @brief  Flags: align + memory info          */
.long -(0x1BADB002 + 0x03)    /* @brief  Checksum (must sum to zero)        */

/* ========================================================================== */
/*  Boot GDT — 4 entries (null, 32-bit code, data, 64-bit code)              */
/* ========================================================================== */
.align 16
gdt64:
.quad 0x0000000000000000      /* @brief  Null descriptor                    */
.quad 0x00CF9A000000FFFF      /* @brief  32-bit ring-0 code segment         */
.quad 0x00CF92000000FFFF      /* @brief  Ring-0 data segment                */
.quad 0x00AF9A000000FFFF      /* @brief  64-bit ring-0 code segment (L=1)   */
gdt64_ptr:
.word 31                      /* @brief  GDT size - 1                       */
.long gdt64                   /* @brief  GDT linear base address            */

/**
 * @brief   Kernel entry point invoked by GRUB
 * @details Sets up long mode: identity page tables (PML4 → PDPT → PD with
 *          2 MiB huge page), PAE, LM-bit in EFER MSR, paging enable (CR0.PG),
 *          then loads the boot GDT and far-jumps into 64-bit code.
 * @warning Interrupts must remain disabled (CLI) until the IDT is installed.
 */
.global _start
_start:
    cli

    /* ---- Zero out page-table region (PML4 + PDPT + PD = 12 KiB) -------- */
    mov $pml4, %edi
    xor %eax, %eax
    mov $(4096*3), %ecx
    rep stosb

    /* ---- PML4[0] → PDPT (present + writable) --------------------------- */
    mov $pdpt, %eax
    or $0x03, %eax
    mov %eax, pml4

    /* ---- PDPT[0] → PD (present + writable) ----------------------------- */
    mov $pd, %eax
    or $0x03, %eax
    mov %eax, pdpt

    /* ---- PD[0] → 2 MiB huge page (PS=1, present, writable, accessed) -- */
    mov $0x83, %eax
    mov %eax, pd

    /* ---- Load PML4 base into CR3 --------------------------------------- */
    mov $pml4, %eax
    mov %eax, %cr3

    /* ---- Enable PAE (CR4.PAE = bit 5) ---------------------------------- */
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4

    /* ---- Set EFER.LME (long-mode enable) -------------------------------- */
    mov $0xC0000080, %ecx /* IA32_EFER MSR */
    rdmsr
    or $0x100, %eax       /* LME = bit 8 */
    wrmsr

    /* ---- Enable paging (CR0.PG) — CPU is now in compatibility mode ----- */
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0

    /* ---- Load boot GDT and set up data segments ------------------------ */
    lgdt gdt64_ptr
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* ---- Far jump into 64-bit mode (selector 0x18 = our 64-bit CS) ----- */
    ljmp $0x18, $_start64

/* ========================================================================== */
/*  Section: .text — 64-bit kernel entry point                                */
/* ========================================================================== */
.code64
.global _start64
_start64:
    mov $stack_top, %rsp      /* @brief  Set up kernel stack                 */
    call kernel_main           /* @brief  Jump to C kernel_main()            */
    cli
1:  hlt
    jmp 1b

/* ========================================================================== */
/*  Macros: ISR / IRQ stubs                                                   */
/* ========================================================================== */

/**
 * @brief   Generate an exception stub (no error code pushed by CPU)
 * @param   n  Interrupt vector number (0–31)
 * @details Pushes a dummy error code (0), then the vector number, then jumps
 *          to isr_common_stub.
 */
.macro isr_noerr n
.global isr\n
.type isr\n, @function
isr\n:
    push $0                   /* dummy error code                            */
    push $\n                  /* interrupt vector number                     */
    jmp isr_common_stub
.endm

/**
 * @brief   Generate an exception stub (CPU pushes error code)
 * @param   n  Interrupt vector number (8, 10–14, 17)
 * @details Pushes only the vector number, then jumps to isr_common_stub.
 *          The error code is already on the stack from the CPU.
 */
.macro isr_err n
.global isr\n
.type isr\n, @function
isr\n:
    push $\n                  /* interrupt vector number                     */
    jmp isr_common_stub
.endm

/**
 * @brief   Generate an IRQ stub for PIC interrupts
 * @param   n  IRQ number (0–15)
 * @details Pushes a dummy error code, then (32 + n) as the vector number,
 *          then jumps to irq_common_stub. IRQs are remapped by the PIC to
 *          vectors 32–47.
 */
.macro irq_stub n
.global irq\n
.type irq\n, @function
irq\n:
    push $0                   /* dummy error code                            */
    push $(32 + \n)           /* remapped interrupt vector                   */
    jmp irq_common_stub
.endm

/* ---- ISR stubs for CPU exceptions (vectors 0–31) ------------------------- */
isr_noerr 0       /* @brief  Division-by-zero                              */
isr_noerr 1       /* @brief  Debug                                         */
isr_noerr 2       /* @brief  Non-maskable interrupt                        */
isr_noerr 3       /* @brief  Breakpoint                                    */
isr_noerr 4       /* @brief  Overflow                                      */
isr_noerr 5       /* @brief  Bound range                                   */
isr_noerr 6       /* @brief  Invalid opcode                                */
isr_noerr 7       /* @brief  Device not available                          */
isr_err   8       /* @brief  Double fault (has error code)                 */
isr_noerr 9       /* @brief  Coprocessor segment overrun                   */
isr_err   10      /* @brief  Invalid TSS (has error code)                  */
isr_err   11      /* @brief  Segment not present (has error code)          */
isr_err   12      /* @brief  Stack-segment fault (has error code)          */
isr_err   13      /* @brief  General protection fault (has error code)     */
isr_err   14      /* @brief  Page fault (has error code)                   */
isr_noerr 15      /* @brief  Reserved                                      */
isr_noerr 16      /* @brief  x87 FPU error                                 */
isr_err   17      /* @brief  Alignment check (has error code)              */
isr_noerr 18      /* @brief  Machine check                                 */
isr_noerr 19      /* @brief  SIMD FPU exception                            */
isr_noerr 20      /* @brief  Virtualization exception                      */
isr_noerr 21      /* @brief  Control-flow protection                       */
isr_noerr 22      /* @brief  Reserved                                      */
isr_noerr 23      /* @brief  Reserved                                      */
isr_noerr 24      /* @brief  Reserved                                      */
isr_noerr 25      /* @brief  Reserved                                      */
isr_noerr 26      /* @brief  Reserved                                      */
isr_noerr 27      /* @brief  Reserved                                      */
isr_noerr 28      /* @brief  Reserved                                      */
isr_noerr 29      /* @brief  Reserved                                      */
isr_noerr 30      /* @brief  Reserved                                      */
isr_noerr 31      /* @brief  Reserved                                      */

/* ---- IRQ stubs for PIC hardware interrupts (vectors 32–47) --------------- */
irq_stub 0        /* @brief  PIT / system timer (IRQ0)                     */
irq_stub 1        /* @brief  Keyboard (IRQ1)                               */
irq_stub 2        /* @brief  Cascade (IRQ2)                                */
irq_stub 3        /* @brief  COM2 (IRQ3)                                   */
irq_stub 4        /* @brief  COM1 (IRQ4)                                   */
irq_stub 5        /* @brief  LPT2 (IRQ5)                                   */
irq_stub 6        /* @brief  Floppy disk (IRQ6)                            */
irq_stub 7        /* @brief  LPT1 / spurious (IRQ7)                        */
irq_stub 8        /* @brief  CMOS RTC (IRQ8)                               */
irq_stub 9        /* @brief  ACPI / generic (IRQ9)                         */
irq_stub 10       /* @brief  SCSI / NIC (IRQ10)                            */
irq_stub 11       /* @brief  SCSI / NIC (IRQ11)                            */
irq_stub 12       /* @brief  PS/2 mouse (IRQ12)                            */
irq_stub 13       /* @brief  FPU / co-processor (IRQ13)                    */
irq_stub 14       /* @brief  Primary ATA (IRQ14)                           */
irq_stub 15       /* @brief  Secondary ATA (IRQ15)                         */

/* ========================================================================== */
/*  ISR stub table — array of function pointers for C access                  */
/* ========================================================================== */
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

/* ========================================================================== */
/*  Common ISR handler — dispatch to isr_handler() in C                       */
/* ========================================================================== */
.text
isr_common_stub:
    /* ---- Save all general-purpose registers ----------------------------- */
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

    /* ---- Load kernel data segment selectors ----------------------------- */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    cld
    push %rsp                 /* @brief  Pass registers_t pointer to C      */
    call isr_handler
    add $8, %rsp              /* @brief  Pop the pointer                    */

    /* ---- Restore general-purpose registers ------------------------------ */
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
    add $16, %rsp             /* @brief  Pop error code + int vector        */
    iretq

/* ========================================================================== */
/*  Common IRQ handler — dispatch to irq_handler() in C                       */
/* ========================================================================== */
irq_common_stub:
    /* ---- Save all general-purpose registers ----------------------------- */
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

    /* ---- Load kernel data segment selectors ----------------------------- */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    cld
    push %rsp                 /* @brief  Pass registers_t pointer to C      */
    call irq_handler
    add $8, %rsp              /* @brief  Pop the pointer                    */

    /* ---- Restore general-purpose registers ------------------------------ */
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
    add $16, %rsp             /* @brief  Pop error code + int vector        */
    iretq

/* ========================================================================== */
/*  BSS — page tables and kernel stack                                        */
/* ========================================================================== */
.section .bss
.align 4096
pml4:   .skip 4096           /* @brief  Page-map level-4 table (PML4)       */
pdpt:   .skip 4096           /* @brief  Page-directory pointer table (PDPT) */
pd:     .skip 4096           /* @brief  Page directory (PD) — 2 MiB pages   */
stack_bottom:
.skip 16384                   /* @brief  16 KiB kernel stack                 */
stack_top:
