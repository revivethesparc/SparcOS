#include "idt.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256] __attribute__((aligned(16)));
static struct idt_ptr   ip;

void idt_set_entry(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_mid  = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].sel       = sel;
    idt[num].ist       = 0;
    idt[num].flags     = flags;
    idt[num].reserved  = 0;
}

static void idt_load(void) {
    __asm__ volatile ("lidt (%0)" : : "r"(&ip) : "memory");
}

void idt_init(void) {
    ip.limit = sizeof(idt) - 1;
    ip.base  = (uint64_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, 0, 0, 0);
    }

    idt_load();
}
