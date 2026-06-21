#include "keyboard.h"
#include "isr.h"
#include "ports.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT  0x60
#define KEYBOARD_STATUS_PORT 0x64

static char key_buffer[256];
static int  key_buffer_head = 0;
static int  key_buffer_tail = 0;

static const char scancode_ascii[] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=',  0,   0,   'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,   0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

void keyboard_handler(registers_t* regs) {
    (void)regs;

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

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

void keyboard_init(void) {
    while (inb(KEYBOARD_STATUS_PORT) & 1) {
        inb(KEYBOARD_DATA_PORT);
    }
}

char keyboard_getchar(void) {
    if (key_buffer_head == key_buffer_tail) {
        return 0;
    }
    char c = key_buffer[key_buffer_tail];
    key_buffer_tail = (key_buffer_tail + 1) % sizeof(key_buffer);
    return c;
}
