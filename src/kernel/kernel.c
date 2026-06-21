#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "pic.h"
#include "keyboard.h"

void kernel_main(void) {
    terminal_initialize();

    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("OS Framework v0.2 (x86_64)\n");
    terminal_writestring("============================\n\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    terminal_writestring("[OK] VGA text mode initialized\n");
    terminal_writestring("[OK] Long mode enabled via boot.S\n");

    gdt_init();
    terminal_writestring("[OK] GDT initialized (64-bit)\n");

    idt_init();
    terminal_writestring("[OK] IDT initialized\n");

    for (int i = 0; i < 32; i++)
        idt_set_entry(i, isr_stubs[i], 0x08, 0x8E);
    for (int i = 0; i < 16; i++)
        idt_set_entry(32 + i, irq_stubs[i], 0x08, 0x8E);

    terminal_writestring("[OK] ISR handlers installed (0-31)\n");
    terminal_writestring("[OK] IRQ handlers installed (32-47)\n");

    pic_init();
    terminal_writestring("[OK] PIC initialized (IRQs remapped to 32-47)\n");

    keyboard_init();
    isr_register_handler(33, keyboard_handler);
    pic_unmask_irq(1);
    terminal_writestring("[OK] Keyboard initialized\n");

    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nSystem ready. Type something: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    __asm__ volatile("sti");

    while (1) {
        __asm__ volatile("hlt");
    }
}
