#include "paging.h"
#include "types.h"

/* Identity-map the full 4 GiB address space using 4 MiB pages (PSE).
 * This keeps the linear framebuffer mapped no matter where the bootloader
 * placed it, which a 4 KiB identity map of low memory would not. A real
 * demand-paged VM with per-process address spaces is a later milestone. */

static uint32_t page_directory[1024] __attribute__((aligned(4096)));

#define PDE_PRESENT   0x001
#define PDE_RW        0x002
#define PDE_PAGESIZE  0x080   /* 4 MiB page */

void paging_init(void) {
    for (uint32_t i = 0; i < 1024; i++)
        page_directory[i] = (i * 0x400000) | PDE_PRESENT | PDE_RW | PDE_PAGESIZE;

    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x00000010;                       /* CR4.PSE */
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

    __asm__ volatile("mov %0, %%cr3" : : "r"(page_directory));

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;                       /* CR0.PG */
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}
