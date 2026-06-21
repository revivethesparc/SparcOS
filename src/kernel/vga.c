/**
 * @file    vga.c
 * @brief   VGA text-mode terminal driver
 * @details Implements a simple 80×25 VGA text-mode framebuffer driver.
 *          Supports ASCII printable characters, newline, carriage-return,
 *          tab, scrolling, colour changes, and hex output.
 * @note    The VGA framebuffer lives at physical address 0xB8000 and is
 *          composed of 16-bit cells: [character byte | colour byte].
 * @author  OS Framework Team
 * @date    2026
 */

#include "vga.h"
#include "ports.h"

static const size_t VGA_WIDTH  = 80;   /* @brief  Columns                    */
static const size_t VGA_HEIGHT = 25;   /* @brief  Rows                       */
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t       terminal_row;      /* @brief  Current cursor row         */
static size_t       terminal_column;   /* @brief  Current cursor column      */
static uint8_t      terminal_color;    /* @brief  Active foreground/back.    */
static uint16_t*    terminal_buffer;   /* @brief  Pointer to VGA memory      */

/**
 * @brief   Initialise the terminal
 * @details Clears the entire screen with the default colour (light grey on
 *          black) and resets the cursor to (0, 0).
 */
void terminal_initialize(void) {
    terminal_row    = 0;
    terminal_column = 0;
    terminal_color  = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

/**
 * @brief   Set the terminal foreground and background colour
 * @param[in] color  VGA colour byte (foreground in low nibble, background
 *                   in high nibble) as produced by vga_entry_color()
 */
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

/**
 * @brief   Scroll the terminal content up by one line
 * @details Shifts every row up by one and clears the bottom row using the
 *          current colour. Called automatically when the cursor passes the
 *          last row.
 */
void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t from = y * VGA_WIDTH + x;
            const size_t to   = (y - 1) * VGA_WIDTH + x;
            terminal_buffer[to] = terminal_buffer[from];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

/**
 * @brief   Write a single character to the terminal
 * @param[in] c  ASCII character to display
 * @details Handles '\\n' (newline), '\\r' (carriage return), and '\\t' (tab).
 *          Ordinary printable characters are placed at the cursor position.
 *          The cursor advances; when it reaches the right edge or bottom, the
 *          screen scrolls automatically.
 */
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = VGA_HEIGHT - 1;
            terminal_scroll();
        }
        return;
    }

    if (c == '\r') {
        terminal_column = 0;
        return;
    }

    if (c == '\t') {
        terminal_column = (terminal_column + 8) & ~7;
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_row = VGA_HEIGHT - 1;
                terminal_scroll();
            }
        }
        return;
    }

    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = vga_entry((unsigned char)c, terminal_color);

    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = VGA_HEIGHT - 1;
            terminal_scroll();
        }
    }
}

/**
 * @brief   Write a fixed-length buffer of characters to the terminal
 * @param[in] data  Pointer to the character buffer
 * @param[in] size  Number of characters to write
 */
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

/**
 * @brief   Write a null-terminated string to the terminal
 * @param[in] data  Pointer to the null-terminated string
 */
void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

/**
 * @brief   Write a 64-bit unsigned integer in hexadecimal
 * @param[in] num  Value to display
 * @details Formats the value as "0x" + up to 16 hex digits, stripping
 *          leading zeros (except the last digit).
 */
void terminal_write_hex(uint64_t num) {
    const char* hex_digits = "0123456789ABCDEF";
    terminal_write("0x", 2);
    int started = 0;
    for (int i = 60; i >= 0; i -= 4) {
        int digit = (num >> i) & 0xF;
        if (digit || started || i == 0) {
            started = 1;
            terminal_putchar(hex_digits[digit]);
        }
    }
}
