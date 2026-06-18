#ifndef JUPITER_KEYBOARD_H
#define JUPITER_KEYBOARD_H

#include "types.h"

/* special keys delivered through the same byte stream (non-ASCII codes) */
#define KEY_UP    0x11
#define KEY_DOWN  0x12
#define KEY_LEFT  0x13
#define KEY_RIGHT 0x14

void keyboard_init(void);
/* returns next char from the ring buffer, or 0 if empty (non-blocking) */
char keyboard_getchar(void);

#endif
