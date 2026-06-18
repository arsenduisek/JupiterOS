#include "heap.h"

/* First-fit free-list allocator over a static arena. Blocks carry a header
 * with size + free flag and are coalesced on free. This is the simple stage;
 * a buddy + slab allocator (per the spec) is a later milestone. */

#define HEAP_SIZE (4 * 1024 * 1024)   /* 4 MiB */
#define ALIGN8(x) (((x) + 7u) & ~7u)

typedef struct block {
    size_t        size;       /* payload size */
    bool          free;
    struct block *next;
} block_t;

static uint8_t  arena[HEAP_SIZE] __attribute__((aligned(16)));
static block_t *head;
static size_t   used_bytes;

void heap_init(void) {
    head        = (block_t *)arena;
    head->size  = HEAP_SIZE - sizeof(block_t);
    head->free  = true;
    head->next  = NULL;
    used_bytes  = 0;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = ALIGN8(size);

    for (block_t *b = head; b; b = b->next) {
        if (!b->free || b->size < size) continue;

        /* split if there's room for another header + a little payload */
        if (b->size >= size + sizeof(block_t) + 8) {
            block_t *split = (block_t *)((uint8_t *)b + sizeof(block_t) + size);
            split->size = b->size - size - sizeof(block_t);
            split->free = true;
            split->next = b->next;
            b->size = size;
            b->next = split;
        }
        b->free = false;
        used_bytes += b->size + sizeof(block_t);
        return (uint8_t *)b + sizeof(block_t);
    }
    return NULL;   /* out of memory */
}

void kfree(void *ptr) {
    if (!ptr) return;
    block_t *b = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    b->free = true;
    used_bytes -= b->size + sizeof(block_t);

    /* coalesce forward with adjacent free blocks */
    for (block_t *c = head; c; c = c->next) {
        while (c->free && c->next && c->next->free) {
            c->size += sizeof(block_t) + c->next->size;
            c->next = c->next->next;
        }
    }
}

size_t heap_used(void) { return used_bytes; }
