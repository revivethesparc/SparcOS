#include "isr.h"
#include "vga.h"
#include "ports.h"

static isr_t interrupt_handlers[256];

static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_register_handler(uint8_t num, isr_t handler) {
    interrupt_handlers[num] = handler;
}

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

void irq_handler(registers_t* regs) {
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}
