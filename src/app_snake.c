#include "gui.h"
#include "fb.h"
#include "font.h"
#include "keyboard.h"
#include "kprintf.h"

#define CELL 16
#define MAX_CELLS 2048

typedef struct { int x, y; } cell_t;

static cell_t  body[MAX_CELLS];
static int     length;
static int     dir_x, dir_y;
static cell_t  food;
static int     grid_w, grid_h;
static int     score;
static bool    dead;
static bool    started;
static uint32_t last_move;
static uint32_t rng = 2463534242u;

static uint32_t rand_next(void) {
    rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
    return rng;
}

static void place_food(void) {
    for (;;) {
        food.x = rand_next() % grid_w;
        food.y = rand_next() % grid_h;
        bool on_body = false;
        for (int i = 0; i < length; i++)
            if (body[i].x == food.x && body[i].y == food.y) { on_body = true; break; }
        if (!on_body) return;
    }
}

void snake_reset(int w, int h) {
    grid_w = w / CELL;
    grid_h = h / CELL;
    if (grid_w > 60) grid_w = 60;
    if (grid_h > 30) grid_h = 30;
    length = 3;
    body[0] = (cell_t){ grid_w / 2,     grid_h / 2 };
    body[1] = (cell_t){ grid_w / 2 - 1, grid_h / 2 };
    body[2] = (cell_t){ grid_w / 2 - 2, grid_h / 2 };
    dir_x = 1; dir_y = 0;
    score = 0;
    dead = false;
    started = true;
    last_move = 0;
    place_food();
}

void snake_key(char c) {
    if (dead && (c == 'r' || c == 'R' || c == '\n')) {
        snake_reset(grid_w * CELL, grid_h * CELL);
        return;
    }
    /* prevent 180-degree reversal */
    switch (c) {
        case KEY_UP:    case 'w': case 'W': if (dir_y == 0) { dir_x = 0;  dir_y = -1; } break;
        case KEY_DOWN:  case 's': case 'S': if (dir_y == 0) { dir_x = 0;  dir_y =  1; } break;
        case KEY_LEFT:  case 'a': case 'A': if (dir_x == 0) { dir_x = -1; dir_y =  0; } break;
        case KEY_RIGHT: case 'd': case 'D': if (dir_x == 0) { dir_x =  1; dir_y =  0; } break;
    }
}

void snake_update(uint32_t now) {
    if (!started || dead) return;
    if (now - last_move < 9) return;     /* ~9 ticks @100Hz -> ~11 moves/sec */
    last_move = now;

    cell_t head = body[0];
    head.x += dir_x;
    head.y += dir_y;

    if (head.x < 0 || head.y < 0 || head.x >= grid_w || head.y >= grid_h) { dead = true; return; }
    for (int i = 0; i < length; i++)
        if (body[i].x == head.x && body[i].y == head.y) { dead = true; return; }

    bool grow = (head.x == food.x && head.y == food.y);
    if (grow) { score++; if (length < MAX_CELLS) length++; }

    for (int i = length - 1; i > 0; i--) body[i] = body[i - 1];
    body[0] = head;

    if (grow) place_food();
}

static void itoa_u(int v, char *out) {
    char tmp[16]; int i = 0;
    if (v == 0) tmp[i++] = '0';
    while (v) { tmp[i++] = '0' + (v % 10); v /= 10; }
    int k = 0; while (i--) out[k++] = tmp[i]; out[k] = 0;
}

void snake_draw(int x, int y, int w, int h) {
    (void)w; (void)h;
    if (!started) snake_reset(grid_w ? grid_w * CELL : 480, grid_h ? grid_h * CELL : 320);

    int field_w = grid_w * CELL, field_h = grid_h * CELL;
    fb_fill_rect(x, y, field_w, field_h, RGB(0x10, 0x18, 0x10));
    fb_rect(x, y, field_w, field_h, RGB(0x40, 0x80, 0x40));

    fb_fill_rect(x + food.x * CELL + 2, y + food.y * CELL + 2,
                 CELL - 4, CELL - 4, RGB(0xE0, 0x40, 0x40));

    for (int i = 0; i < length; i++) {
        uint32_t col = (i == 0) ? RGB(0x90, 0xFF, 0x90) : RGB(0x40, 0xC0, 0x40);
        fb_fill_rect(x + body[i].x * CELL + 1, y + body[i].y * CELL + 1,
                     CELL - 2, CELL - 2, col);
    }

    char line[32]; char num[16];
    itoa_u(score, num);
    int k = 0; const char *p = "Score: ";
    while (*p) line[k++] = *p++;
    p = num; while (*p) line[k++] = *p++; line[k] = 0;
    font_draw_string(x + 4, y + field_h + 6, line, RGB(0xFF, 0xFF, 0xFF));
    font_draw_string(x + 4, y + field_h + 6 + font_height() + 2,
                     "WASD / arrows to move", RGB(0xC0, 0xC0, 0xC0));

    if (dead) {
        font_draw_string(x + field_w / 2 - 70, y + field_h / 2 - 8,
                         "GAME OVER - press R", RGB(0xFF, 0xE0, 0x40));
    }
}
