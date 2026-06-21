/**
 * @file    gdt.h
 * @brief   Global Descriptor Table — public interface
 * @details Declares the gdt_init() function used to set up the kernel-mode
 *          GDT and reload segment selectors.
 * @author  OS Framework Team
 * @date    2026
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/**
 * @brief   Initialise and install the kernel GDT
 * @details Creates a 5-entry GDT with null, kernel code/data, and user
 *          code/data segments, then performs a GDT flush to reload all
 *          segment registers including CS (via far return).
 */
void gdt_init(void);

#endif
