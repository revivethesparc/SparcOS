/**
 * @file    vga.h
 * @brief   VGA text-mode terminal — public interface
 * @details Provides colour constants, utility inline functions for composing
 *          VGA entry bytes, and the full terminal API for writing to the
 *          80×25 text-mode display.
 * @note    Include this header in any module that needs console output.
 * @author  OS Framework Team
 * @date    2026
 */

#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief   Standard VGA text-mode colour palette
 * @info    Each colour is a 4-bit value used in the VGA attribute byte.
 *          Foreground occupies the low nibble; background the high nibble.
 */
enum vga_color {
    VGA_COLOR_BLACK         = 0,   /* @brief  0: #000000                    */
    VGA_COLOR_BLUE          = 1,   /* @brief  1: #0000AA                    */
    VGA_COLOR_GREEN         = 2,   /* @brief  2: #00AA00                    */
    VGA_COLOR_CYAN          = 3,   /* @brief  3: #00AAAA                    */
    VGA_COLOR_RED           = 4,   /* @brief  4: #AA0000                    */
    VGA_COLOR_MAGENTA       = 5,   /* @brief  5: #AA00AA                    */
    VGA_COLOR_BROWN         = 6,   /* @brief  6: #AA5500                    */
    VGA_COLOR_LIGHT_GREY    = 7,   /* @brief  7: #AAAAAA                    */
    VGA_COLOR_DARK_GREY     = 8,   /* @brief  8: #555555                    */
    VGA_COLOR_LIGHT_BLUE    = 9,   /* @brief  9: #5555FF                    */
    VGA_COLOR_LIGHT_GREEN   = 10,  /* @brief  A: #55FF55                    */
    VGA_COLOR_LIGHT_CYAN    = 11,  /* @brief  B: #55FFFF                    */
    VGA_COLOR_LIGHT_RED     = 12,  /* @brief  C: #FF5555                    */
    VGA_COLOR_LIGHT_MAGENTA = 13,  /* @brief  D: #FF55FF                    */
    VGA_COLOR_LIGHT_BROWN   = 14,  /* @brief  E: #FFFF55                    */
    VGA_COLOR_WHITE         = 15,  /* @brief  F: #FFFFFF                    */
};

/* ---- Public terminal API ------------------------------------------------- */

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_write_hex(uint64_t num);

/* ---- Inline helpers ------------------------------------------------------ */

/**
 * @brief   Combine foreground and background colours into a VGA attribute byte
 * @param[in] fg  Foreground colour (@ref vga_color)
 * @param[in] bg  Background colour (@ref vga_color)
 * @return  uint8_t attribute byte: [bg:4 | fg:4]
 */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

/**
 * @brief   Build a VGA framebuffer cell from a character and colour attribute
 * @param[in] uc     ASCII character code
 * @param[in] color  VGA attribute byte from vga_entry_color()
 * @return  uint16_t cell value: [color:8 | char:8]
 */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

#endif
