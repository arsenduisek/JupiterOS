#ifndef JUPITER_FB_H
#define JUPITER_FB_H

#include "types.h"
#include "multiboot.h"

#define RGB(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

typedef struct {
    uint32_t *addr;       /* back buffer we draw into */
    uint32_t *front;      /* hardware framebuffer */
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch_px;   /* pitch in pixels */
    uint8_t   bpp;
    bool      active;
} framebuffer_t;

extern framebuffer_t fb;

/* returns false if no usable 32bpp linear framebuffer was provided */
bool fb_init(multiboot_info_t *mbi);
void fb_clear(uint32_t color);
void fb_pixel(int x, int y, uint32_t color);
void fb_fill_rect(int x, int y, int w, int h, uint32_t color);
void fb_rect(int x, int y, int w, int h, uint32_t color);   /* outline */
void fb_present(void);   /* copy back buffer -> screen */

#endif
