/* Multiboot 1 header + 32-bit entry point.
 * Requests a linear framebuffer (1024x768x32) via the video fields. */

.set ALIGN,    1 << 0              /* align loaded modules on page boundaries */
.set MEMINFO,  1 << 1              /* provide a memory map */
.set VIDINFO,  1 << 2              /* request a video mode (framebuffer) */
.set FLAGS,    ALIGN | MEMINFO | VIDINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0      /* header_addr    (unused, ELF) */
.long 0      /* load_addr      (unused, ELF) */
.long 0      /* load_end_addr  (unused, ELF) */
.long 0      /* bss_end_addr   (unused, ELF) */
.long 0      /* entry_addr     (unused, ELF) */
.long 0      /* mode_type: 0 = linear graphics framebuffer */
.long 1024   /* width  */
.long 768    /* height */
.long 32     /* depth (bits per pixel) */

.section .bss
.align 16
stack_bottom:
.skip 16384                        /* 16 KiB kernel stack */
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    movl $stack_top, %esp
    pushl %ebx                     /* multiboot info structure pointer */
    pushl %eax                     /* multiboot magic value */
    call kmain
.hang:
    cli
    hlt
    jmp .hang
.size _start, . - _start
