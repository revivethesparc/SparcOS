#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef struct {
    uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags;
    uint64_t rsp, ss;
} __attribute__((packed)) registers_t;

typedef void (*isr_t)(registers_t*);

void isr_handler(registers_t* regs);
void irq_handler(registers_t* regs);
void isr_register_handler(uint8_t num, isr_t handler);

extern uint64_t isr_stubs[32];
extern uint64_t irq_stubs[16];

#endif
