#ifndef JUPITER_ISR_H
#define JUPITER_ISR_H

#include "types.h"

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;  /* pusha */
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;                  /* pushed by CPU */
} registers_t;

typedef void (*irq_handler_t)(registers_t *);

void idt_init(void);
void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void register_irq_handler(uint8_t irq, irq_handler_t handler);

#endif
