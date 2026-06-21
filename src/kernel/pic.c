/**
 * @file    pic.c
 * @brief   Programmable Interrupt Controller — Intel 8259A driver
 * @details Implements initialisation and masking for the master and slave
 *          PICs. The PIC is programmed in 8086 mode with IRQs remapped from
 *          their default vectors (0–15) to vectors 32–47 to avoid overlap
 *          with CPU exceptions.
 * @note    After initialisation all IRQs are masked (0xFF). Specific IRQs
 *          must be unmasked via pic_unmask_irq() to enable the device.
 * @author  OS Framework Team
 * @date    2026
 */

#include "pic.h"
#include "ports.h"

/* ---- PIC port addresses ------------------------------------------------- */
#define PIC1        0x20      /* @brief  Master PIC command port              */
#define PIC2        0xA0      /* @brief  Slave PIC command port               */
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1 + 1)   /* @brief  Master PIC data port           */
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2 + 1)   /* @brief  Slave PIC data port            */

/* ---- Initialisation command words (ICW) --------------------------------- */
#define ICW1_INIT    0x11     /* @brief  ICW1: initialise + expect ICW4      */
#define ICW1_ICW4    0x01     /* @brief  ICW1: ICW4 will be sent             */
#define ICW4_8086    0x01     /* @brief  ICW4: 8086 mode                     */

/* ---- Remapped interrupt base vectors ------------------------------------ */
#define PIC1_OFFSET  0x20     /* @brief  Master IRQ 0–7 → vectors 32–39     */
#define PIC2_OFFSET  0x28     /* @brief  Slave IRQ 8–15 → vectors 40–47     */

/**
 * @brief   Initialise both PICs in 8086 mode
 * @details Sends the full ICW sequence (ICW1–ICW4) to the master and slave:
 *          - ICW1: Initialise, indicate ICW4 follows
 *          - ICW2: Remap base vector offset
 *          - ICW3: Slave/master cascade wiring
 *          - ICW4: Set 8086 mode
 *          After setup all IRQ lines are masked (0xFF on both data ports).
 * @warning Requires io_wait() between commands for legacy bus compatibility.
 */
void pic_init(void) {
    /* ICW1: begin initialisation */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* ICW2: set vector offset */
    outb(PIC1_DATA, PIC1_OFFSET);
    io_wait();
    outb(PIC2_DATA, PIC2_OFFSET);
    io_wait();

    /* ICW3: cascade wiring — slave at IRQ2 on master */
    outb(PIC1_DATA, 0x04);   /* @brief  Master: slave at IRQ2 (bit 2)       */
    io_wait();
    outb(PIC2_DATA, 0x02);   /* @brief  Slave: cascade identity (value 2)   */
    io_wait();

    /* ICW4: 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Mask all IRQs */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

/**
 * @brief   Mask (disable) a specific IRQ line
 * @param[in] irq  IRQ number (0–15)
 * @note    The PIC masks IRQs by setting the corresponding bit in the
 *          appropriate data port's Interrupt Mask Register (IMR).
 */
void pic_mask_irq(uint8_t irq) {
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = irq < 8 ? irq : irq - 8;
    outb(port, inb(port) | (1 << bit));
}

/**
 * @brief   Unmask (enable) a specific IRQ line
 * @param[in] irq  IRQ number (0–15)
 * @note    This must be called after pic_init() for each IRQ the kernel
 *          intends to handle.
 */
void pic_unmask_irq(uint8_t irq) {
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = irq < 8 ? irq : irq - 8;
    outb(port, inb(port) & ~(1 << bit));
}
