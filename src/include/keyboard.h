#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "isr.h"

void keyboard_init(void);
void keyboard_handler(registers_t* regs);
char keyboard_getchar(void);

#endif
