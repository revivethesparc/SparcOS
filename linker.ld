/**
 * @file    linker.ld
 * @brief   Kernel linker script — x86_64 ELF layout
 * @details Defines the memory layout of the kernel binary:
 *          - Load address: 1 MiB (standard for GRUB Multiboot)
 *          - .text32:     32-bit entry code (Multiboot header + boot GDT)
 *          - .text:       64-bit kernel code
 *          - .rodata:     Read-only data (strings, stub tables)
 *          - .data:       Initialised data
 *          - .bss:        Zero-initialised data (page tables, stack)
 * @note    The Multiboot header MUST reside in .text32 to guarantee it
 *          falls within the first PT_LOAD segment and thus within the
 *          first 8192 bytes GRUB scans.
 * @author  OS Framework Team
 * @date    2026
 */

ENTRY(_start)

SECTIONS {
    /* @brief  Load base — GRUB will place us at 1 MiB                       */
    . = 1M;

    /* @brief  32-bit bootstrap section (Multiboot header, entry, boot GDT) */
    .text32 : {
        *(.text32)
    }

    /* @brief  64-bit kernel code section                                   */
    .text : {
        *(.text)
    }

    /* @brief  Read-only data                                                */
    .rodata : {
        *(.rodata)
    }

    /* @brief  Initialised data                                              */
    .data : {
        *(.data)
    }

    /* @brief  Zero-initialised data (BSS)                                   */
    .bss : {
        *(COMMON)
        *(.bss)
    }
}
