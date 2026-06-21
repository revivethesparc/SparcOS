/**
 * @file    gdt.c
 * @brief   Global Descriptor Table — x86_64 ring-0/ring-3 segment setup
 * @details Implements a 5-entry GDT with null, kernel code/data, and user
 *          code/data segments. The boot GDT created in boot.S (which contains
 *          only 32-bit and 64-bit code segments) is replaced here.
 * @note    In long mode the GDT controls only data-segment attributes and
 *          privilege levels; code-segment descriptors are largely ignored
 *          except for DPL and the L-bit.
 * @warning The GDT flush uses a far return (lretq) to reload the code segment
 *          selector (CS). The stack must be valid before calling gdt_flush().
 * @author  OS Framework Team
 * @date    2026
 */

#include "gdt.h"

/**
 * @brief   Packed GDT entry (8 bytes)
 * @details Standard x86 GDT descriptor format.
 * @see     Intel SDM Vol. 3A §3.4.5
 */
struct gdt_entry {
    uint16_t limit_low;      /* @brief  Limit bits 0–15                     */
    uint16_t base_low;       /* @brief  Base bits 0–15                      */
    uint8_t  base_middle;    /* @brief  Base bits 16–23                     */
    uint8_t  access;         /* @brief  Access byte (present, DPL, type…)   */
    uint8_t  granularity;    /* @brief  Flags + limit bits 16–19            */
    uint8_t  base_high;      /* @brief  Base bits 24–31                     */
} __attribute__((packed));

/**
 * @brief   GDT pointer structure for LGDT instruction
 */
struct gdt_ptr {
    uint16_t limit;          /* @brief  Size of GDT minus one               */
    uint64_t base;           /* @brief  64-bit linear address of GDT        */
} __attribute__((packed));

static struct gdt_entry gdt[5];   /* @brief  Kernel-mode GDT table           */
static struct gdt_ptr   gp;       /* @brief  GDT pointer for LGDT            */

/**
 * @brief   Populate a single GDT descriptor
 * @param[in] num     Entry index in gdt[]
 * @param[in] base    32-bit segment base address (ignored in 64-bit mode)
 * @param[in] limit   20-bit segment limit (ignored for code in 64-bit mode)
 * @param[in] access  Access-rights byte
 * @param[in] gran    Granularity and size flags (G, D/B, L, AVL)
 * @note    In long mode base and limit are ignored for code segments, but
 *          must still be encoded correctly for data segments.
 */
static void gdt_set_entry(int num, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran) {
    gdt[num].base_low     = base & 0xFFFF;
    gdt[num].base_middle  = (base >> 16) & 0xFF;
    gdt[num].base_high    = (base >> 24) & 0xFF;
    gdt[num].limit_low    = limit & 0xFFFF;
    gdt[num].granularity  = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access       = access;
}

/**
 * @brief   Load the new GDT and reload segment selectors
 * @details Executes LGDT, reloads data segments (DS/ES/FS/GS/SS) with the
 *          ring-0 data selector (0x10), and performs a far return through
 *          the ring-0 code selector (0x08) to pick up the new CS.
 * @note    The `lea 1f(%%rip), %%rax` trick computes the return address
 *          RIP-relative so the far return lands on the next instruction.
 */
static void gdt_flush(void) {
    __asm__ volatile (
        "lgdt (%0)\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "push $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        : : "r"(&gp) : "rax", "memory"
    );
}

/**
 * @brief   Initialise and load the kernel GDT
 * @details Sets up a 5-entry GDT:
 *          - Entry 0: Null descriptor (required)
 *          - Entry 1: Kernel-mode code segment (ring 0, 64-bit, DPL=0)
 *          - Entry 2: Kernel-mode data segment (ring 0, DPL=0)
 *          - Entry 3: User-mode code segment (ring 3, 64-bit, DPL=3)
 *          - Entry 4: User-mode data segment (ring 3, DPL=3)
 * @note    Selector values: 0x08, 0x10, 0x18, 0x20.
 */
void gdt_init(void) {
    gp.limit = sizeof(gdt) - 1;
    gp.base  = (uint64_t)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);              /* Null                      */
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xAF);  /* Kernel code (ring 0)      */
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);  /* Kernel data (ring 0)      */
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xAF);  /* User code (ring 3)        */
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xCF);  /* User data (ring 3)        */

    gdt_flush();
}
