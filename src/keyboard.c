#include "keyboard.h"
#include "isr.h"
#include "io.h"

#define KBD_DATA 0x60

static const char map_lower[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/',  0,
    '*', 0, ' ',
};

static const char map_upper[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A','S','D','F','G','H','J','K','L',':','"','~',
    0,   '|','Z','X','C','V','B','N','M','<','>','?',  0,
    '*', 0, ' ',
};

#define BUF_SIZE 256
static volatile char buf[BUF_SIZE];
static volatile uint32_t head, tail;

static bool shift, ctrl, extended;

static void buf_push(char c) {
    uint32_t next = (head + 1) % BUF_SIZE;
    if (next != tail) { buf[head] = c; head = next; }
}

char keyboard_getchar(void) {
    if (tail == head) return 0;
    char c = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    return c;
}

static void keyboard_handler(registers_t *r) {
    (void)r;
    uint8_t sc = inb(KBD_DATA);

    if (sc == 0xE0) { extended = true; return; }

    bool release = sc & 0x80;
    uint8_t code = sc & 0x7F;

    if (extended) {
        extended = false;
        if (!release) {
            switch (code) {
                case 0x48: buf_push(KEY_UP);    break;
                case 0x50: buf_push(KEY_DOWN);  break;
                case 0x4B: buf_push(KEY_LEFT);  break;
                case 0x4D: buf_push(KEY_RIGHT); break;
            }
        }
        return;
    }

    if (release) {
        if (code == 0x2A || code == 0x36) shift = false;
        else if (code == 0x1D) ctrl = false;
        return;
    }

    switch (code) {
        case 0x2A: case 0x36: shift = true; return;
        case 0x1D: ctrl = true; return;
        case 0x3A: shift = !shift; return;   /* caps lock (simplified) */
        default: break;
    }

    char c = shift ? map_upper[code] : map_lower[code];
    if (c) buf_push(c);
}

void keyboard_init(void) {
    head = tail = 0;
    shift = ctrl = extended = false;
    register_irq_handler(1, keyboard_handler);
}
