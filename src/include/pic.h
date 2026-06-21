/**
 * @file    pic.h
 * @brief   Programmable Interrupt Controller — public interface
 * @details Declares the PIC driver API: initialisation (with IRQ remapping
 *          to vectors 32–47) and per-IRQ masking/unmasking.
 * @author  OS Framework Team
 * @date    2026
 */

#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/**
 * @brief   Initialise the master and slave PICs in 8086 mode
 * @details Sends ICW1–ICW4 to both PICs, remapping IRQ 0–7 to vectors 32–39
 *          and IRQ 8–15 to vectors 40–47. All IRQs are masked afterwards.
 */
void pic_init(void);

/**
 * @brief   Disable a specific IRQ line
 * @param[in] irq  IRQ number (0–15)
 */
void pic_mask_irq(uint8_t irq);

/**
 * @brief   Enable a specific IRQ line
 * @param[in] irq  IRQ number (0–15)
 * @note    Interrupts remain blocked at the CPU until STI is executed.
 */
void pic_unmask_irq(uint8_t irq);

#endif
