#include "mouse.h"
#include "isr.h"
#include "io.h"

#define PS2_DATA   0x60
#define PS2_STATUS 0x64
#define PS2_CMD    0x64

static int px, py, bound_x, bound_y;
static uint8_t buttons;

static uint8_t cycle;
static int8_t  packet[3];

static void ps2_wait_input(void) {            /* wait until we can write */
    for (int i = 0; i < 100000; i++)
        if (!(inb(PS2_STATUS) & 0x02)) return;
}

static void ps2_wait_output(void) {           /* wait until data available */
    for (int i = 0; i < 100000; i++)
        if (inb(PS2_STATUS) & 0x01) return;
}

static void mouse_write(uint8_t val) {
    ps2_wait_input(); outb(PS2_CMD, 0xD4);    /* address the aux device */
    ps2_wait_input(); outb(PS2_DATA, val);
}

static uint8_t mouse_read(void) {
    ps2_wait_output();
    return inb(PS2_DATA);
}

static void mouse_handler(registers_t *r) {
    (void)r;
    uint8_t status = inb(PS2_STATUS);
    if (!(status & 0x20)) return;             /* not from the aux device */

    int8_t data = (int8_t)inb(PS2_DATA);
    switch (cycle) {
        case 0:
            if (!(data & 0x08)) return;        /* resync: bit3 always set in byte0 */
            packet[0] = data; cycle = 1; break;
        case 1:
            packet[1] = data; cycle = 2; break;
        case 2:
            packet[2] = data; cycle = 0;
            buttons = packet[0] & 0x07;
            px += packet[1];
            py -= packet[2];                   /* screen Y grows downward */
            if (px < 0) px = 0;
            if (py < 0) py = 0;
            if (px > bound_x) px = bound_x;
            if (py > bound_y) py = bound_y;
            break;
    }
}

int     mouse_x(void)       { return px; }
int     mouse_y(void)       { return py; }
uint8_t mouse_buttons(void) { return buttons; }

void mouse_init(int max_x, int max_y) {
    bound_x = max_x; bound_y = max_y;
    px = max_x / 2; py = max_y / 2;
    cycle = 0; buttons = 0;

    ps2_wait_input(); outb(PS2_CMD, 0xA8);     /* enable aux device */

    ps2_wait_input(); outb(PS2_CMD, 0x20);     /* read controller config */
    uint8_t cfg = mouse_read();
    cfg |= 0x02;                               /* enable IRQ12 */
    cfg &= ~0x20;                              /* enable aux clock */
    ps2_wait_input(); outb(PS2_CMD, 0x60);     /* write controller config */
    ps2_wait_input(); outb(PS2_DATA, cfg);

    mouse_write(0xF6); mouse_read();           /* set defaults (ACK) */
    mouse_write(0xF4); mouse_read();           /* enable data reporting (ACK) */

    register_irq_handler(12, mouse_handler);
}
