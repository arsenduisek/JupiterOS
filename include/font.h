#ifndef JUPITER_FONT_H
#define JUPITER_FONT_H

#include "types.h"

void font_init(void);
int  font_width(void);
int  font_height(void);
void font_draw_char(int x, int y, char c, uint32_t fg);
void font_draw_string(int x, int y, const char *s, uint32_t fg);

#endif
