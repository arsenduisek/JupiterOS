#include "gui.h"
#include "fb.h"
#include "font.h"
#include "heap.h"
#include "timer.h"
#include "kprintf.h"

#define ROWS 24
#define COLS 96

static char  screen[ROWS][COLS];
static int   cur_row, cur_col;
static char  input[COLS];
static int   inlen;

#define COL_BG   RGB(0x0A, 0x0A, 0x12)
#define COL_FG   RGB(0xC8, 0xF0, 0xC8)

static void term_clear(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) screen[r][c] = ' ';
    cur_row = cur_col = 0;
}

static void term_scroll(void) {
    for (int r = 1; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) screen[r - 1][c] = screen[r][c];
    for (int c = 0; c < COLS; c++) screen[ROWS - 1][c] = ' ';
    cur_row = ROWS - 1;
}

static void term_newline(void) {
    cur_col = 0;
    if (++cur_row >= ROWS) term_scroll();
}

static void term_putc(char c) {
    if (c == '\n') { term_newline(); return; }
    if (c == '\b') { if (cur_col > 0) { cur_col--; screen[cur_row][cur_col] = ' '; } return; }
    if (cur_col >= COLS) term_newline();
    screen[cur_row][cur_col++] = c;
}

static void term_print(const char *s) { while (*s) term_putc(*s++); }

static void term_print_u(uint32_t v) {
    char tmp[16]; int i = 0;
    if (v == 0) { term_putc('0'); return; }
    while (v) { tmp[i++] = '0' + (v % 10); v /= 10; }
    while (i--) term_putc(tmp[i]);
}

static void prompt(void) { term_print("jupiter$ "); }

static void execute(char *cmd) {
    /* split off the first token */
    char *arg = cmd;
    while (*arg && *arg != ' ') arg++;
    while (*arg == ' ') { *arg = 0; arg++; }   /* terminate verb, skip spaces */

    if (cmd[0] == 0) {
        return;
    } else if (strcmp(cmd, "help") == 0) {
        term_print("Commands:\n");
        term_print("  help    - this list\n");
        term_print("  echo X  - print X\n");
        term_print("  clear   - clear screen\n");
        term_print("  ver     - version info\n");
        term_print("  mem     - heap usage\n");
        term_print("  uptime  - seconds since boot\n");
        term_print("  ls      - list files (stub)\n");
        term_print("  snake   - launch the snake game\n");
        term_print("  about   - about JupiterOS\n");
    } else if (strcmp(cmd, "echo") == 0) {
        term_print(arg); term_putc('\n');
    } else if (strcmp(cmd, "clear") == 0) {
        term_clear();
    } else if (strcmp(cmd, "ver") == 0 || strcmp(cmd, "version") == 0) {
        term_print("JupiterOS 0.1 (i386, multiboot)\n");
    } else if (strcmp(cmd, "mem") == 0) {
        term_print("heap used: "); term_print_u(heap_used()); term_print(" bytes\n");
    } else if (strcmp(cmd, "uptime") == 0) {
        term_print("up "); term_print_u(timer_ticks() / 100); term_print(" s\n");
    } else if (strcmp(cmd, "ls") == 0) {
        term_print("(no filesystem mounted yet - ext2 is a later milestone)\n");
    } else if (strcmp(cmd, "snake") == 0) {
        term_print("launching snake...\n");
        gui_set_app(APP_SNAKE);
    } else if (strcmp(cmd, "about") == 0) {
        term_print("JupiterOS - a from-scratch x86 OS.\n");
        term_print("kernel, paging, heap, VESA GUI, PS/2 input.\n");
    } else {
        term_print(cmd); term_print(": command not found (try 'help')\n");
    }
}

void terminal_init(void) {
    term_clear();
    inlen = 0;
    term_print("JupiterOS shell - type 'help'\n\n");
    prompt();
}

void terminal_key(char c) {
    if (c == '\n') {
        term_putc('\n');
        input[inlen] = 0;
        execute(input);
        inlen = 0;
        prompt();
    } else if (c == '\b') {
        if (inlen > 0) { inlen--; term_putc('\b'); }
    } else if (c >= 32 && c < 127 && inlen < COLS - 1) {
        input[inlen++] = c;
        term_putc(c);
    }
}

void terminal_draw(int x, int y, int w, int h) {
    fb_fill_rect(x, y, w, h, COL_BG);
    int fw = font_width(), fh = font_height();
    int vis_rows = h / fh; if (vis_rows > ROWS) vis_rows = ROWS;
    int vis_cols = w / fw; if (vis_cols > COLS) vis_cols = COLS;

    for (int r = 0; r < vis_rows; r++) {
        for (int c = 0; c < vis_cols; c++) {
            char ch = screen[r][c];
            if (ch != ' ') font_draw_char(x + c * fw, y + r * fh, ch, COL_FG);
        }
    }
    /* draw a block cursor */
    if (cur_row < vis_rows && cur_col < vis_cols)
        fb_fill_rect(x + cur_col * fw, y + cur_row * fh + fh - 2, fw, 2, COL_FG);
}
