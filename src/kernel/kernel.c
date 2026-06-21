/**
 * @file    kernel.c
 * @brief   Kernel entry point and initialization sequence
 * @details This file implements kernel_main(), the first C function called
 *          after the assembly bootstrap completes. It initialises subsystems
 *          in order: VGA text-mode terminal, GDT, IDT, PIC, and keyboard.
 *          After setup it enables interrupts (STI) and idles in a HLT loop.
 * @info    The boot GDT from boot.S is replaced by a full 5-entry GDT with
 *          ring-0/ring-3 segments set up in gdt_init().
 * @note    ISR stubs (vectors 0–47) are installed from assembly arrays
 *          isr_stubs[] and irq_stubs[].
 * @author  OS Framework Team
 * @date    2026
 */

#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "pic.h"
#include "keyboard.h"

#include "func/print_hello.h"
#include "func/print_version.h"


/**
 * @brief   Kernel main entry point
 * @details Initialises all hardware subsystems in dependency order, installs
 *          interrupt handlers, then enables interrupts and halts. All further
 *          system activity is driven by IRQ handlers (keyboard, timer, etc.).
 * @info    Called from boot.S _start64 with a valid stack pointer.
 * @warning This function never returns — it loops indefinitely.
 */
void kernel_main(void) {
    /* ---- Step 1: VGA text-mode terminal --------------------------------- */
    terminal_initialize();

    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("OS Framework v0.2 (x86_64)\n");
    terminal_writestring("============================\n\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    terminal_writestring("[OK] VGA text mode initialized\n");
    terminal_writestring("[OK] Long mode enabled via boot.S\n");

    /* ---- Step 2: Global Descriptor Table (GDT) -------------------------- */
    gdt_init();
    terminal_writestring("[OK] GDT initialized (64-bit)\n");

    /* ---- Step 3: Interrupt Descriptor Table (IDT) ----------------------- */
    idt_init();
    terminal_writestring("[OK] IDT initialized\n");

    /* ---- Step 4: Install ISR / IRQ stub vectors 0–47 -------------------- */
    for (int i = 0; i < 32; i++)
        idt_set_entry(i, isr_stubs[i], 0x08, 0x8E);
    for (int i = 0; i < 16; i++)
        idt_set_entry(32 + i, irq_stubs[i], 0x08, 0x8E);

    terminal_writestring("[OK] ISR handlers installed (0-31)\n");
    terminal_writestring("[OK] IRQ handlers installed (32-47)\n");

    /* ---- Step 5: Programmable Interrupt Controller (PIC) ---------------- */
    pic_init();
    terminal_writestring("[OK] PIC initialized (IRQs remapped to 32-47)\n");

    /* ---- Step 6: Keyboard ----------------------------------------------- */
    keyboard_init();
    isr_register_handler(33, keyboard_handler);
    pic_unmask_irq(1);
    terminal_writestring("[OK] Keyboard initialized\n");

    /* ---- Enable interrupts and idle ------------------------------------- */
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nSystem ready. Type something: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    print_hello();
    print_version();

    __asm__ volatile("sti");

    while (1) {
        __asm__ volatile("hlt");
    }
}
