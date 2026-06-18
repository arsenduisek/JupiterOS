#include "types.h"
#include "multiboot.h"
#include "vga.h"
#include "serial.h"
#include "kprintf.h"
#include "gdt.h"
#include "isr.h"
#include "paging.h"
#include "heap.h"
#include "keyboard.h"
#include "timer.h"
#include "mouse.h"
#include "fb.h"
#include "font.h"
#include "gui.h"

void kmain(uint32_t magic, uint32_t mb_info) {
    multiboot_info_t *mbi = (multiboot_info_t *)mb_info;

    serial_init();
    vga_init();
    kprintf("JupiterOS boot (magic=%x)\n", magic);

    kprintf("GDT...\n");    gdt_init();
    kprintf("IDT...\n");    idt_init();
    kprintf("PIC...\n");    pic_remap();
    kprintf("Paging...\n"); paging_init();
    kprintf("Heap...\n");   heap_init();

    kprintf("Framebuffer... ");
    if (fb_init(mbi)) {
        kprintf("%ux%ux%u @ %p\n", fb.width, fb.height, fb.bpp, (uint32_t)fb.front);
        font_init();
        timer_init(100);
        keyboard_init();
        mouse_init(fb.width - 1, fb.height - 1);
        __asm__ volatile("sti");
        kprintf("Entering GUI.\n");
        gui_run();                       /* never returns */
    }

    /* ---- text-mode fallback (no framebuffer provided) ---- */
    kprintf("none; staying in VGA text mode.\n");
    timer_init(100);
    keyboard_init();
    __asm__ volatile("sti");
    kprintf("Type away:\n\n");
    for (;;) {
        char c = keyboard_getchar();
        if (c) kputc(c);
        else   __asm__ volatile("hlt");
    }
}
