#include "gui.h"
#include "fb.h"
#include "font.h"
#include "mouse.h"
#include "keyboard.h"
#include "timer.h"

#define COL_DESKTOP   RGB(0x20, 0x60, 0x80)
#define COL_TASKBAR   RGB(0x18, 0x18, 0x20)
#define COL_WIN_BODY  RGB(0xF0, 0xF0, 0xF0)
#define COL_WIN_TITLE RGB(0x30, 0x50, 0xC0)
#define COL_WHITE     RGB(0xFF, 0xFF, 0xFF)
#define COL_BORDER    RGB(0x00, 0x00, 0x00)
#define COL_BTN       RGB(0x30, 0x80, 0x40)
#define COL_BTN_HI    RGB(0x40, 0xA0, 0x55)

#define TITLE_H   22
#define TASKBAR_H 28

typedef struct { int x, y, w, h; const char *title; } window_t;

static window_t win = { 180, 120, 600, 440, "Terminal" };
static int      active = APP_TERMINAL;
static int      pending = -1;

void gui_set_app(int app) { pending = app; }

static void content_rect(int *cx, int *cy, int *cw, int *ch) {
    *cx = win.x + 1;
    *cy = win.y + TITLE_H;
    *cw = win.w - 2;
    *ch = win.h - TITLE_H - 1;
}

static const char *cursor_glyph[] = {
    "#", "##", "#.#", "#..#", "#...#", "#....#", "#.....#", "#......#",
    "#.......#", "#........#", "#.....####", "#..#..#", "#.# #..#",
    "##  #..#", "#    #..#", "     #..#", "      #.#", "      ###",
};
#define CURSOR_ROWS (int)(sizeof(cursor_glyph) / sizeof(cursor_glyph[0]))

static void draw_cursor(int mx, int my) {
    for (int row = 0; row < CURSOR_ROWS; row++) {
        const char *line = cursor_glyph[row];
        for (int col = 0; line[col]; col++)
            if (line[col] == '#') {
                fb_pixel(mx + col, my + row, COL_BORDER);
                if (line[col + 1] == '.') fb_pixel(mx + col + 1, my + row, COL_WHITE);
            }
    }
}

/* taskbar buttons */
typedef struct { const char *label; int app; int x, w; } button_t;
static button_t buttons[] = {
    { "Terminal", APP_TERMINAL, 6,   96 },
    { "Snake",    APP_SNAKE,    108, 80 },
    { "About",    APP_ABOUT,    194, 80 },
};
#define NBUTTONS (int)(sizeof(buttons) / sizeof(buttons[0]))

static void switch_to(int app) {
    active = app;
    int cx, cy, cw, ch;
    content_rect(&cx, &cy, &cw, &ch);
    if (app == APP_TERMINAL) win.title = "Terminal";
    else if (app == APP_SNAKE) { win.title = "Snake"; snake_reset(cw, ch - 40); }
    else if (app == APP_ABOUT) win.title = "About";
}

static void draw_about(int x, int y, int w, int h) {
    fb_fill_rect(x, y, w, h, COL_WIN_BODY);
    uint32_t fg = RGB(0x10, 0x10, 0x10);
    int fh = font_height();
    font_draw_string(x + 14, y + 16,                "JupiterOS  v0.1", RGB(0x20,0x40,0xA0));
    font_draw_string(x + 14, y + 16 + 2*fh,         "A from-scratch x86 operating system.", fg);
    font_draw_string(x + 14, y + 16 + 3*fh + 4,     "Multiboot kernel, paging, heap,", fg);
    font_draw_string(x + 14, y + 16 + 4*fh + 4,     "IDT/PIC/IRQ, PS/2 keyboard+mouse,", fg);
    font_draw_string(x + 14, y + 16 + 5*fh + 4,     "VESA framebuffer, PSF2 fonts, WM.", fg);
    font_draw_string(x + 14, y + 16 + 7*fh + 4,     "Apps: Terminal (shell) + Snake.", fg);
    font_draw_string(x + 14, y + 16 + 9*fh + 4,     "Use the taskbar to switch apps.", RGB(0x40,0x40,0x40));
}

static void draw_window(void) {
    fb_fill_rect(win.x, win.y, win.w, TITLE_H, COL_WIN_TITLE);
    fb_rect(win.x, win.y, win.w, win.h, COL_BORDER);
    font_draw_string(win.x + 8, win.y + (TITLE_H - font_height()) / 2, win.title, COL_WHITE);

    int cx, cy, cw, ch;
    content_rect(&cx, &cy, &cw, &ch);
    if (active == APP_TERMINAL)   terminal_draw(cx, cy, cw, ch);
    else if (active == APP_SNAKE) snake_draw(cx, cy, cw, ch);
    else                          draw_about(cx, cy, cw, ch);
}

static void draw_taskbar(void) {
    int y = fb.height - TASKBAR_H;
    fb_fill_rect(0, y, fb.width, TASKBAR_H, COL_TASKBAR);
    for (int i = 0; i < NBUTTONS; i++) {
        bool on = (buttons[i].app == active);
        fb_fill_rect(buttons[i].x, y + 4, buttons[i].w, TASKBAR_H - 8,
                     on ? COL_BTN_HI : COL_BTN);
        font_draw_string(buttons[i].x + 8, y + (TASKBAR_H - font_height()) / 2,
                         buttons[i].label, COL_WHITE);
    }
    /* uptime clock */
    char clk[24]; int i = 0; uint32_t t = timer_ticks() / 100; char tmp[12]; int n = 0;
    if (t == 0) tmp[n++] = '0';
    while (t) { tmp[n++] = '0' + (t % 10); t /= 10; }
    const char *p = "up ";
    while (*p) clk[i++] = *p++;
    while (n--) clk[i++] = tmp[n];
    clk[i++] = 's';
    clk[i] = 0;
    font_draw_string(fb.width - 90, y + (TASKBAR_H - font_height()) / 2, clk, COL_WHITE);
}

void gui_run(void) {
    terminal_init();

    bool dragging = false, prev_left = false;
    int  drag_dx = 0, drag_dy = 0;

    for (;;) {
        if (pending >= 0) { switch_to(pending); pending = -1; }

        int mx = mouse_x(), my = mouse_y();
        bool left = mouse_buttons() & MOUSE_LEFT;
        int tb_y = fb.height - TASKBAR_H;

        if (left && !prev_left) {
            /* taskbar button click */
            bool hit_button = false;
            if (my >= tb_y) {
                for (int i = 0; i < NBUTTONS; i++)
                    if (mx >= buttons[i].x && mx < buttons[i].x + buttons[i].w) {
                        switch_to(buttons[i].app); hit_button = true; break;
                    }
            }
            /* titlebar drag */
            if (!hit_button && mx >= win.x && mx < win.x + win.w &&
                my >= win.y && my < win.y + TITLE_H) {
                dragging = true; drag_dx = mx - win.x; drag_dy = my - win.y;
            }
        }
        if (!left) dragging = false;
        prev_left = left;

        if (dragging) { win.x = mx - drag_dx; win.y = my - drag_dy; }

        /* route keyboard to the active app */
        char c;
        while ((c = keyboard_getchar()) != 0) {
            if (active == APP_SNAKE) snake_key(c);
            else if (active == APP_TERMINAL) terminal_key(c);
        }

        if (active == APP_SNAKE) snake_update(timer_ticks());

        /* render */
        fb_clear(COL_DESKTOP);
        font_draw_string(16, 12, "JupiterOS", COL_WHITE);
        draw_window();
        draw_taskbar();
        draw_cursor(mx, my);
        fb_present();

        __asm__ volatile("hlt");
    }
}
