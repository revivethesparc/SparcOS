/**
 * @file    isr.h
 * @brief   Interrupt service routines — types, externs, and public API
 * @details Defines the registers_t structure (CPU register snapshot), the
 *          isr_t callback typedef, and the public ISR/IRQ interface.
 *          Also declares the assembly stub-address arrays isr_stubs[] and
 *          irq_stubs[] defined in boot.S.
 * @author  OS Framework Team
 * @date    2026
 */

#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/**
 * @brief   CPU register snapshot pushed by ISR/IRQ common stubs
 * @details The order of fields matches the push sequence in the assembly
 *          stubs (rax at the lowest address, ss at the highest). The
 *          structure is packed so that offset calculations in C match the
 *          stack layout exactly.
 * @warning Fields must stay in this order — changing it breaks register
 *          restoration in isr_common_stub / irq_common_stub.
 */
typedef struct {
    /* Manually pushed (in push order) */
    uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    /* Pushed by the stub */
    uint64_t int_no, err_code;
    /* Pushed by the CPU on interrupt entry */
    uint64_t rip, cs, rflags;
    uint64_t rsp, ss;
} __attribute__((packed)) registers_t;

/**
 * @brief   Callback type for interrupt handlers
 * @param[in,out] regs  Pointer to the saved register state
 */
typedef void (*isr_t)(registers_t*);

/* ---- Public API ---------------------------------------------------------- */

void isr_handler(registers_t* regs);
void irq_handler(registers_t* regs);
void isr_register_handler(uint8_t num, isr_t handler);

/**
 * @brief   Address table for assembly ISR stubs (vectors 0–31)
 * @details Defined in boot.S via .irp expansion. Each element is the
 *          64-bit absolute address of an isrXX label.
 */
extern uint64_t isr_stubs[32];

/**
 * @brief   Address table for assembly IRQ stubs (IRQ 0–15 → vectors 32–47)
 * @details Defined in boot.S via .irp expansion. Each element is the
 *          64-bit absolute address of an irqXX label.
 */
extern uint64_t irq_stubs[16];

#endif
