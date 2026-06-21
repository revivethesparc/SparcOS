/**
 * @file    ports.h
 * @brief   x86 port I/O — inline assembly helpers
 * @details Provides portable inline functions for reading and writing to
 *          x86 I/O ports (in/out instructions). Includes byte and word
 *          variants plus a short I/O delay helper.
 * @note    All functions are static inline — no linker footprint when unused.
 * @author  OS Framework Team
 * @date    2026
 */

#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

/**
 * @brief   Write a byte to the specified I/O port
 * @param[in] port  Target I/O port address (0–65535)
 * @param[in] val   Byte value to write
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/**
 * @brief   Read a byte from the specified I/O port
 * @param[in] port  Source I/O port address (0–65535)
 * @return  Byte value read from the port
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief   Write a word (16 bits) to the specified I/O port
 * @param[in] port  Target I/O port address (0–65535)
 * @param[in] val   Word value to write
 */
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

/**
 * @brief   Read a word (16 bits) from the specified I/O port
 * @param[in] port  Source I/O port address (0–65535)
 * @return  Word value read from the port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief   Short I/O delay for bus settling
 * @details Writes to the unused 0x80 port (POST status), which creates a
 *          delay of ~1 µs on typical hardware. Required between some PIC
 *          and PIT command sequences on legacy systems.
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif
