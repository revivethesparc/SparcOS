#pragma once
#include "base.h"
#include "vga.h"

FUNC(print_version, "Prints the kernel version string", "1.0.0") {
    terminal_writestring("OS Framework v0.0.2\n");
}