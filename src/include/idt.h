/**
 * @file    idt.h
 * @brief   Interrupt Descriptor Table — public interface
 * @details Declares functions for IDT initialisation and entry configuration.
 * @author  OS Framework Team
 * @date    2026
 */

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/**
 * @brief   Initialise the IDT and load it via LIDT
 * @details Zeros all 256 entries and loads the IDT base/limit into the
 *          IDTR. Individual vectors should be set up afterwards with
 *          idt_set_entry().
 */
void idt_init(void);

/**
 * @brief   Set an IDT entry for a given interrupt vector
 * @param[in] num    Vector index (0–255)
 * @param[in] base   64-bit address of the handler function
 * @param[in] sel    Code segment selector (typically 0x08)
 * @param[in] flags  Gate type and attributes (e.g. 0x8E = interrupt gate)
 */
void idt_set_entry(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);

#endif
