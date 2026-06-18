#include "font.h"
#include "fb.h"

/* Embedded PSF2 font (see src/font_data.c). */
extern const unsigned char font_psf2_data[];
extern const unsigned int  font_psf2_len;

struct psf2_header {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
};

#define PSF2_MAGIC 0x864ab572

static const struct psf2_header *hdr;
static const uint8_t            *glyphs;
static int g_width = 8, g_height = 16, g_bpg = 16;

void font_init(void) {
    hdr = (const struct psf2_header *)font_psf2_data;
    if (hdr->magic == PSF2_MAGIC) {
        g_width  = hdr->width;
        g_height = hdr->height;
        g_bpg    = hdr->bytesperglyph;
        glyphs   = font_psf2_data + hdr->headersize;
    } else {
        glyphs = font_psf2_data + 32;   /* fall back to fixed 8x16 layout */
    }
}

int font_width(void)  { return g_width; }
int font_height(void) { return g_height; }

void font_draw_char(int x, int y, char c, uint32_t fg) {
    unsigned ch = (unsigned char)c;
    if (ch >= 256) ch = '?';
    const uint8_t *g = glyphs + ch * g_bpg;
    int bytes_per_row = (g_width + 7) / 8;
    for (int row = 0; row < g_height; row++) {
        for (int col = 0; col < g_width; col++) {
            const uint8_t *rowp = g + row * bytes_per_row;
            if (rowp[col / 8] & (0x80 >> (col % 8)))
                fb_pixel(x + col, y + row, fg);
        }
    }
}

void font_draw_string(int x, int y, const char *s, uint32_t fg) {
    int cx = x;
    for (; *s; s++) {
        if (*s == '\n') { cx = x; y += g_height; continue; }
        font_draw_char(cx, y, *s, fg);
        cx += g_width;
    }
}
