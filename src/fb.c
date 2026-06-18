#include "fb.h"
#include "heap.h"
#include "kprintf.h"

framebuffer_t fb;

/* double-buffer storage; if the heap can't satisfy it we draw straight to VRAM */
static uint32_t *backbuffer;

bool fb_init(multiboot_info_t *mbi) {
    if (!(mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER))
        return false;
    if (mbi->framebuffer_bpp != 32 || mbi->framebuffer_type != 1)
        return false;   /* we only handle 32bpp direct-color RGB */

    fb.front    = (uint32_t *)(uint32_t)mbi->framebuffer_addr;
    fb.width    = mbi->framebuffer_width;
    fb.height   = mbi->framebuffer_height;
    fb.pitch_px = mbi->framebuffer_pitch / 4;
    fb.bpp      = 32;

    backbuffer = kmalloc(fb.pitch_px * fb.height * 4);
    fb.addr    = backbuffer ? backbuffer : fb.front;
    fb.active  = true;
    return true;
}

void fb_clear(uint32_t color) {
    uint32_t n = fb.pitch_px * fb.height;
    for (uint32_t i = 0; i < n; i++) fb.addr[i] = color;
}

void fb_pixel(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || (uint32_t)x >= fb.width || (uint32_t)y >= fb.height) return;
    fb.addr[y * fb.pitch_px + x] = color;
}

void fb_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            fb_pixel(x + i, y + j, color);
}

void fb_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < w; i++) { fb_pixel(x + i, y, color); fb_pixel(x + i, y + h - 1, color); }
    for (int j = 0; j < h; j++) { fb_pixel(x, y + j, color); fb_pixel(x + w - 1, y + j, color); }
}

void fb_present(void) {
    if (fb.addr == fb.front) return;   /* no back buffer; already on screen */
    uint32_t n = fb.pitch_px * fb.height;
    for (uint32_t i = 0; i < n; i++) fb.front[i] = fb.addr[i];
}
