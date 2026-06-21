/**
 * @file    keyboard.c
 * @brief   PS/2 keyboard driver
 * @details Implements a simple PS/2 keyboard driver that reads scancodes
 *          from port 0x60, translates them to ASCII via a lookup table, and
 *          buffers the characters in a circular buffer. The handler is
 *          registered as an IRQ1 (vector 33) callback.
 * @note    Only press events (scancode < 0x80) are processed; release events
 *          (high bit set) are silently discarded. No shift/caps-lock support.
 * @author  OS Framework Team
 * @date    2026
 */

#include "keyboard.h"
#include "isr.h"
#include "ports.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT    0x60   /* @brief  PS/2 data register           */
#define KEYBOARD_STATUS_PORT  0x64   /* @brief  PS/2 status register         */

static char key_buffer[256];          /* @brief  Circular character buffer    */
static int  key_buffer_head = 0;      /* @brief  Producer index              */
static int  key_buffer_tail = 0;      /* @brief  Consumer index              */

/**
 * @brief   PS/2 scancode set 1 → ASCII lookup table
 * @details Indexed by the scancode byte received from the keyboard. Entries
 *          are untranslated (byte 0 = scancode 0, which never occurs). Zero
 *          entries indicate unprintable or unmapped keys.
 */
static const char scancode_ascii[] = {
     0,   0,   '1', '2', '3', '4', '5', '6', '7', '8',   /* 0x00–0x09     */
    '9', '0', '-', '=',  0,   0,   'q', 'w', 'e', 'r',   /* 0x0A–0x13     */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,   0,      /* 0x14–0x1D     */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    /* 0x1E–0x27     */
    '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n',  /* 0x28–0x31     */
    'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,      /* 0x32–0x3B     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x3C–0x45     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x46–0x4F     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x50–0x59     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x5A–0x63     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x64–0x6D     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x6E–0x77     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x78–0x81     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x82–0x8B     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x8C–0x95     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0x96–0x9F     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xA0–0xA9     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xAA–0xB3     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xB4–0xBD     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xBE–0xC7     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xC8–0xD1     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xD2–0xDB     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xDC–0xE5     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xE6–0xEF     */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,      /* 0xF0–0xFF     */
};

/**
 * @brief   IRQ1 keyboard interrupt handler
 * @param[in] regs  Register snapshot (unused, suppresses unused-param warning)
 * @details Reads a scancode from the PS/2 data port. If it is a press event
 *          (bit 7 clear) and maps to a printable ASCII character, the
 *          character is written to the VGA terminal and enqueued in the
 *          circular buffer.
 */
void keyboard_handler(registers_t* regs) {
    (void)regs;

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    /* @brief  Ignore release events (scancode with high bit set) */
    if (scancode & 0x80) {
        return;
    }

    if (scancode < sizeof(scancode_ascii)) {
        char c = scancode_ascii[scancode];
        if (c) {
            int next = (key_buffer_head + 1) % sizeof(key_buffer);
            if (next != key_buffer_tail) {
                key_buffer[key_buffer_head] = c;
                key_buffer_head = next;
            }
            terminal_putchar(c);
        }
    }
}

/**
 * @brief   Initialise the PS/2 keyboard controller
 * @details Drains any stale data from the keyboard buffer to avoid
 *          interpreting boot-time noise as a key press.
 */
void keyboard_init(void) {
    while (inb(KEYBOARD_STATUS_PORT) & 1) {
        inb(KEYBOARD_DATA_PORT);
    }
}

/**
 * @brief   Dequeue a character from the keyboard circular buffer
 * @return  The oldest buffered character, or 0 if the buffer is empty.
 * @note    This function is non-blocking — call it in a polling loop or
 *          from a timer tick to consume keyboard input.
 */
char keyboard_getchar(void) {
    if (key_buffer_head == key_buffer_tail) {
        return 0;
    }
    char c = key_buffer[key_buffer_tail];
    key_buffer_tail = (key_buffer_tail + 1) % sizeof(key_buffer);
    return c;
}
