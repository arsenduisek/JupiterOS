#include "isr.h"
#include "io.h"
#include "kprintf.h"

/* ---------------- IDT ---------------- */

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

/* assembly stubs */
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);
extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

static void *const isr_table[32] = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9,
    isr10, isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19,
    isr20, isr21, isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29,
    isr30, isr31,
};

static void *const irq_table[16] = {
    irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
    irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15,
};

static void idt_set_gate(int n, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[n].base_low  = base & 0xFFFF;
    idt[n].base_high = (base >> 16) & 0xFFFF;
    idt[n].selector  = sel;
    idt[n].zero      = 0;
    idt[n].flags     = flags;
}

/* ---------------- PIC ---------------- */

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_CMD  PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_CMD  PIC2
#define PIC2_DATA (PIC2 + 1)

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void pic_remap(void) {
    outb(PIC1_CMD, 0x11); io_wait();   /* start init (cascade) */
    outb(PIC2_CMD, 0x11); io_wait();
    outb(PIC1_DATA, 0x20); io_wait();  /* master offset -> 0x20 */
    outb(PIC2_DATA, 0x28); io_wait();  /* slave offset  -> 0x28 */
    outb(PIC1_DATA, 0x04); io_wait();  /* tell master about slave at IRQ2 */
    outb(PIC2_DATA, 0x02); io_wait();
    outb(PIC1_DATA, 0x01); io_wait();  /* 8086 mode */
    outb(PIC2_DATA, 0x01); io_wait();
    outb(PIC1_DATA, 0x00);             /* unmask all */
    outb(PIC2_DATA, 0x00);
}

/* ---------------- dispatch ---------------- */

static irq_handler_t irq_handlers[16];

void register_irq_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) irq_handlers[irq] = handler;
}

static const char *exception_names[] = {
    "Divide-by-zero", "Debug", "NMI", "Breakpoint", "Overflow",
    "Bound range", "Invalid opcode", "Device not available", "Double fault",
    "Coprocessor overrun", "Invalid TSS", "Segment not present",
    "Stack-segment fault", "General protection fault", "Page fault",
    "Reserved", "x87 FP", "Alignment check", "Machine check", "SIMD FP",
};

void isr_handler(registers_t *r) {
    if (r->int_no < 32) {
        const char *name = (r->int_no < 20) ? exception_names[r->int_no] : "Reserved";
        kprintf("\n*** EXCEPTION %u (%s) err=%x eip=%x ***\n",
                r->int_no, name, r->err_code, r->eip);
        for (;;) __asm__ volatile("cli; hlt");
    } else if (r->int_no < 48) {
        uint8_t irq = r->int_no - 32;
        if (irq_handlers[irq]) irq_handlers[irq](r);
        pic_send_eoi(irq);
    }
}

void idt_init(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;
    memset(&idt, 0, sizeof(idt));

    for (int i = 0; i < 32; i++)
        idt_set_gate(i, (uint32_t)isr_table[i], 0x08, 0x8E);
    for (int i = 0; i < 16; i++)
        idt_set_gate(32 + i, (uint32_t)irq_table[i], 0x08, 0x8E);

    __asm__ volatile("lidt %0" : : "m"(idtp));
}
