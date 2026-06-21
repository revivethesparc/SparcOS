/**
 * @file    idt.c
 * @brief   Interrupt Descriptor Table — x86_64 vector management
 * @details Implements the IDT initialisation and entry setup for 256 vectors.
 *          Each entry is a 16-byte descriptor supporting a 64-bit handler
 *          address, interrupt-stack-table selector, and type/attributes.
 * @note    The IDT is aligned to a 16-byte boundary as required by the
 *          LIDT instruction.
 * @author  OS Framework Team
 * @date    2026
 */

#include "idt.h"

/**
 * @brief   Packed IDT entry (16 bytes) — x86_64 format
 * @details The handler address is split across three fields (low, mid, high)
 *          to form a full 64-bit pointer.
 * @see     Intel SDM Vol. 3A §6.10.1
 */
struct idt_entry {
    uint16_t base_low;       /* @brief  Handler address bits 0–15           */
    uint16_t sel;            /* @brief  Code segment selector               */
    uint8_t  ist;            /* @brief  Interrupt stack table (bits 0–2)    */
    uint8_t  flags;          /* @brief  Type and attributes (present, DPL…) */
    uint16_t base_mid;       /* @brief  Handler address bits 16–31          */
    uint32_t base_high;      /* @brief  Handler address bits 32–63          */
    uint32_t reserved;       /* @brief  Reserved, must be zero              */
} __attribute__((packed));

/**
 * @brief   IDT pointer structure for LIDT instruction
 */
struct idt_ptr {
    uint16_t limit;          /* @brief  Size of IDT minus one               */
    uint64_t base;           /* @brief  64-bit linear address of IDT        */
} __attribute__((packed));

static struct idt_entry idt[256] __attribute__((aligned(16)));
static struct idt_ptr   ip;

/**
 * @brief   Configure a single IDT entry
 * @param[in] num    Interrupt vector number (0–255)
 * @param[in] base   64-bit handler function address
 * @param[in] sel    Code segment selector to use when handling the interrupt
 * @param[in] flags  Gate type, DPL, and present bit (e.g. 0x8E = 32-bit
 *                   interrupt gate, ring 0, present)
 * @details Splits the 64-bit address into three 16/32-bit fields and stores
 *          them in the appropriate descriptor fields. The IST field is set
 *          to zero (no stack switching).
 */
void idt_set_entry(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_mid  = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].sel       = sel;
    idt[num].ist       = 0;
    idt[num].flags     = flags;
    idt[num].reserved  = 0;
}

/**
 * @brief   Execute the LIDT instruction to load the IDT pointer
 */
static void idt_load(void) {
    __asm__ volatile ("lidt (%0)" : : "r"(&ip) : "memory");
}

/**
 * @brief   Initialise the IDT
 * @details Zeros all 256 entries, then loads the IDT pointer via LIDT.
 *          Individual handler vectors are installed separately via
 *          idt_set_entry() after calling this function.
 */
void idt_init(void) {
    ip.limit = sizeof(idt) - 1;
    ip.base  = (uint64_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, 0, 0, 0);
    }

    idt_load();
}
