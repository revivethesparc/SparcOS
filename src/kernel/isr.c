/**
 * @file    isr.c
 * @brief   Interrupt service routines — C dispatch layer
 * @details Provides the C-side handlers for CPU exceptions (ISR 0–31) and
 *          hardware IRQs (32–47). Exceptions trigger a kernel panic with
 *          register dump; IRQs are dispatched through a registered-handler
 *          array and must acknowledge the PIC.
 * @note    The assembly stubs in boot.S save all registers, call these
 *          functions, and restore before iretq.
 * @author  OS Framework Team
 * @date    2026
 */

#include "isr.h"
#include "vga.h"
#include "ports.h"

/**
 * @brief   Registered interrupt-handler callback table
 * @details Indexed by vector number (0–255). A NULL entry means the
 *          interrupt is unhandled at the C level (exceptions still cause
 *          a panic, IRQs are silently acknowledged).
 */
static isr_t interrupt_handlers[256];

/**
 * @brief   Human-readable names for CPU exception vectors 0–31
 */
static const char* exception_messages[] = {
    "Division By Zero",                /* 0                                  */
    "Debug",                           /* 1                                  */
    "Non Maskable Interrupt",          /* 2                                  */
    "Breakpoint",                      /* 3                                  */
    "Into Detected Overflow",          /* 4                                  */
    "Out of Bounds",                   /* 5                                  */
    "Invalid Opcode",                  /* 6                                  */
    "No Coprocessor",                  /* 7                                  */
    "Double Fault",                    /* 8                                  */
    "Coprocessor Segment Overrun",     /* 9                                  */
    "Bad TSS",                         /* 10                                 */
    "Segment Not Present",             /* 11                                 */
    "Stack Fault",                     /* 12                                 */
    "General Protection Fault",        /* 13                                 */
    "Page Fault",                      /* 14                                 */
    "Unknown Interrupt",               /* 15                                 */
    "Coprocessor Fault",               /* 16                                 */
    "Alignment Check",                 /* 17                                 */
    "Machine Check",                   /* 18                                 */
    "Reserved",                        /* 19                                 */
    "Reserved",                        /* 20                                 */
    "Reserved",                        /* 21                                 */
    "Reserved",                        /* 22                                 */
    "Reserved",                        /* 23                                 */
    "Reserved",                        /* 24                                 */
    "Reserved",                        /* 25                                 */
    "Reserved",                        /* 26                                 */
    "Reserved",                        /* 27                                 */
    "Reserved",                        /* 28                                 */
    "Reserved",                        /* 29                                 */
    "Reserved",                        /* 30                                 */
    "Reserved",                        /* 31                                 */
};

/**
 * @brief   Register a C-level handler for an interrupt vector
 * @param[in] num     Vector number (0–255)
 * @param[in] handler Function pointer to call when the interrupt fires
 */
void isr_register_handler(uint8_t num, isr_t handler) {
    interrupt_handlers[num] = handler;
}

/**
 * @brief   Common ISR dispatcher — called from isr_common_stub assembly
 * @param[in,out] regs  Pointer to the saved register state on the stack
 * @details For exception vectors (0–31): prints the exception name, fault
 *          address (if page fault), and register dump, then halts.
 *          For all vectors: invokes any registered C handler.
 * @warning This function never returns for CPU exceptions — it stalls the CPU
 *          with CLI + HLT after printing the panic message.
 */
void isr_handler(registers_t* regs) {
    if (regs->int_no < 32) {
        terminal_writestring("\n[KERNEL PANIC] ");
        terminal_writestring(exception_messages[regs->int_no]);

        if (regs->int_no == 14) {
            uint64_t fault_addr;
            __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));
            terminal_writestring(" at address ");
            terminal_write_hex(fault_addr);
        }

        terminal_writestring("\n");
        terminal_writestring("  RIP: ");
        terminal_write_hex(regs->rip);
        terminal_writestring("  CS: ");
        terminal_write_hex(regs->cs);
        terminal_writestring("  RFLAGS: ");
        terminal_write_hex(regs->rflags);
        terminal_writestring("\n");

        __asm__ volatile("cli; hlt");
    }

    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}

/**
 * @brief   Common IRQ dispatcher — called from irq_common_stub assembly
 * @param[in,out] regs  Pointer to the saved register state on the stack
 * @details Acknowledges the interrupt by sending EOI to the PIC(s).
 *          If the IRQ is from the slave PIC (vector ≥ 40), both PICs receive
 *          the EOI. Then invokes any registered C handler.
 */
void irq_handler(registers_t* regs) {
    /* Send EOI — slave if vector >= 40 */
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}
