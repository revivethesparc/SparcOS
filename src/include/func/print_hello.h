#pragma once
#include "../base.h"
#include "../vga.h"

FUNC(print_hello, "Prints \"Hello\" to the terminal", "1.0.0") {
    terminal_writestring("Hello\n");
}
