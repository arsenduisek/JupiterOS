#include "kprintf.h"
#include "vga.h"
#include "serial.h"
#include <stdarg.h>

/* ---- minimal freestanding string/mem helpers ---- */

void *memset(void *dst, int c, size_t n) {
    uint8_t *d = dst;
    while (n--) *d++ = (uint8_t)c;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

int memcmp(const void *a, const void *b, size_t n) {
    const uint8_t *x = a, *y = b;
    while (n--) { if (*x != *y) return *x - *y; x++; y++; }
    return 0;
}

size_t strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (uint8_t)*a - (uint8_t)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && (*a == *b)) { a++; b++; n--; }
    if (n == 0) return 0;
    return (uint8_t)*a - (uint8_t)*b;
}

/* ---- output sink: screen + serial ---- */

void kputc(char c) {
    vga_putc(c);
    serial_putc(c);
}

static void kputs(const char *s) {
    while (*s) kputc(*s++);
}

static void kput_uint(uint32_t v, uint32_t base, int upper) {
    char buf[33];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    if (v == 0) { kputc('0'); return; }
    while (v) { buf[i++] = digits[v % base]; v /= base; }
    while (i--) kputc(buf[i]);
}

static void kput_int(int32_t v) {
    if (v < 0) { kputc('-'); kput_uint((uint32_t)(-v), 10, 0); }
    else kput_uint((uint32_t)v, 10, 0);
}

void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { kputc(*fmt); continue; }
        fmt++;
        switch (*fmt) {
            case 'c': kputc((char)va_arg(ap, int)); break;
            case 's': kputs(va_arg(ap, const char *)); break;
            case 'd': case 'i': kput_int(va_arg(ap, int32_t)); break;
            case 'u': kput_uint(va_arg(ap, uint32_t), 10, 0); break;
            case 'x': kput_uint(va_arg(ap, uint32_t), 16, 0); break;
            case 'X': kput_uint(va_arg(ap, uint32_t), 16, 1); break;
            case 'p': kputs("0x"); kput_uint(va_arg(ap, uint32_t), 16, 0); break;
            case '%': kputc('%'); break;
            case '\0': va_end(ap); return;
            default:  kputc('%'); kputc(*fmt); break;
        }
    }
    va_end(ap);
}
