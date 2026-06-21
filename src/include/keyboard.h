/**
 * @file    keyboard.h
 * @brief   PS/2 keyboard driver — public interface
 * @details Declares the keyboard initialisation, handler, and character-
 *          retrieval functions.
 * @author  OS Framework Team
 * @date    2026
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "isr.h"

/**
 * @brief   Initialise the PS/2 keyboard controller
 * @details Drains stale data from the keyboard buffer.
 */
void keyboard_init(void);

/**
 * @brief   IRQ1 interrupt handler for keyboard input
 * @param[in] regs  Saved register state (unused)
 */
void keyboard_handler(registers_t* regs);

/**
 * @brief   Read the next buffered character (non-blocking)
 * @return  ASCII character, or 0 if the buffer is empty
 */
char keyboard_getchar(void);

#endif
