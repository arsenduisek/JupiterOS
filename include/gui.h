#ifndef JUPITER_GUI_H
#define JUPITER_GUI_H

#include "types.h"

enum app_id { APP_NONE = 0, APP_TERMINAL, APP_SNAKE, APP_ABOUT };

/* called by apps (e.g. the shell `snake` command) to switch the active app */
void gui_set_app(int app);

void gui_run(void);   /* never returns: desktop event loop */

/* ---- app interface (implemented in app_terminal.c / app_snake.c) ---- */
void terminal_init(void);
void terminal_key(char c);
void terminal_draw(int x, int y, int w, int h);

void snake_reset(int w, int h);
void snake_key(char c);
void snake_update(uint32_t now_ticks);
void snake_draw(int x, int y, int w, int h);

#endif
