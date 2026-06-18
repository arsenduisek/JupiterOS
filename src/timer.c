#include "timer.h"
#include "isr.h"
#include "io.h"

static volatile uint32_t ticks;

static void timer_handler(registers_t *r) {
    (void)r;
    ticks++;
}

uint32_t timer_ticks(void) { return ticks; }

void timer_init(uint32_t hz) {
    ticks = 0;
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36);                       /* channel 0, lobyte/hibyte, mode 3 */
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
    register_irq_handler(0, timer_handler);
}
