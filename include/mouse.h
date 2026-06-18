#ifndef JUPITER_MOUSE_H
#define JUPITER_MOUSE_H

#include "types.h"

#define MOUSE_LEFT   0x01
#define MOUSE_RIGHT  0x02
#define MOUSE_MIDDLE 0x04

void mouse_init(int max_x, int max_y);
int  mouse_x(void);
int  mouse_y(void);
uint8_t mouse_buttons(void);

#endif
