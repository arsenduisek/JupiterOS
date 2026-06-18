#include "vga.h"
#include "io.h"

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  ((volatile uint16_t *)0xB8000)

static size_t   row;
static size_t   col;
static uint8_t  color;

static inline uint16_t vga_entry(char c, uint8_t clr) {
    return (uint16_t)c | ((uint16_t)clr << 8);
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    color = fg | (bg << 4);
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry(' ', color);
    row = 0;
    col = 0;
}

void vga_init(void) {
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
}

static void vga_update_cursor(void) {
    uint16_t pos = row * VGA_WIDTH + col;
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

static void vga_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
    for (size_t x = 0; x < VGA_WIDTH; x++)
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', color);
    row = VGA_HEIGHT - 1;
}

void vga_putc(char c) {
    if (c == '\n') {
        col = 0;
        if (++row >= VGA_HEIGHT) vga_scroll();
    } else if (c == '\r') {
        col = 0;
    } else if (c == '\t') {
        col = (col + 8) & ~7u;
        if (col >= VGA_WIDTH) { col = 0; if (++row >= VGA_HEIGHT) vga_scroll(); }
    } else if (c == '\b') {
        if (col > 0) { col--; VGA_MEMORY[row * VGA_WIDTH + col] = vga_entry(' ', color); }
    } else {
        VGA_MEMORY[row * VGA_WIDTH + col] = vga_entry(c, color);
        if (++col >= VGA_WIDTH) { col = 0; if (++row >= VGA_HEIGHT) vga_scroll(); }
    }
    vga_update_cursor();
}

void vga_write(const char *s) {
    while (*s) vga_putc(*s++);
}
