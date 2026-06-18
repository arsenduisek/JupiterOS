# JupiterOS

A from-scratch x86 operating system, built toward the JUPITEROS spec
(bootloader → kernel → GUI → apps → network → filesystem).

Built with the **host GCC** in 32-bit freestanding mode — no `i686-elf`
cross-compiler required.

## Build

```sh
make            # produces jupiteros.elf (Multiboot 1)
make check      # verify it's a valid multiboot image (needs grub-file)
```

## Run

This environment had no QEMU installed. Install it first (Arch / CachyOS):

```sh
sudo pacman -S qemu-system-x86          # qemu-system-i386 / x86_64
# optional, only needed for an ISO via GRUB:
sudo pacman -S xorriso mtools
```

Then boot the kernel directly (no ISO / GRUB needed — qemu loads multiboot
kernels itself):

```sh
make run        # qemu-system-i386 -kernel jupiteros.elf -serial stdio
```

All console output is mirrored to the serial port, so you see it both in the
QEMU window (VGA text) and in your terminal (serial).

## Debug

```sh
make debug      # starts qemu paused with a GDB stub on :1234
# in a second terminal:
gdb jupiteros.elf -ex 'target remote :1234' -ex 'break kmain' -ex continue
```

## Current status (milestones 1–8: bootable graphical desktop)

- [x] Multiboot 1 header, 32-bit protected-mode entry (`boot.S`)
- [x] VGA text console + COM1 serial logging
- [x] `kprintf` and freestanding `mem*/str*` helpers
- [x] GDT (kernel/user code+data segments)
- [x] IDT + ISR stubs (CPU exceptions) + PIC remap + IRQ dispatch
- [x] PS/2 keyboard driver (scancode set 1, shift/ctrl, ring buffer)
- [x] **Paging** — full 4 GiB identity map via 4 MiB PSE pages (`paging.c`)
- [x] **Heap** — first-fit `kmalloc`/`kfree` with coalescing (`heap.c`)
- [x] **PIT timer** (IRQ0, 100 Hz) + **PS/2 mouse** (IRQ12, 3-byte packets)
- [x] **VESA framebuffer** 1024x768x32, double-buffered (`fb.c`)
- [x] **PSF2 font** rendering — real `default8x16.psfu` embedded + parsed (`font.c`)
- [x] **GUI** — desktop, draggable window w/ titlebar, taskbar launcher, cursor (`gui.c`)
- [x] **Snake** — grid game, food/score, WASD+arrows, game-over/restart (`app_snake.c`)
- [x] **Terminal + shell** — char grid, prompt, built-in commands (`app_terminal.c`)

Shell commands: `help`, `echo`, `clear`, `ver`, `mem`, `uptime`, `ls` (stub),
`snake` (launches the game), `about`.

What you should see when you run it: a teal desktop with a draggable window
and a taskbar with **Terminal / Snake / About** buttons. Terminal opens by
default — type `help`. Click **Snake** (or run `snake` in the shell) to play
with WASD or the arrow keys.

## Run it (one command)

```sh
./run.sh        # installs QEMU if missing (asks sudo), builds, boots
```

or manually:

```sh
sudo pacman -S --needed qemu-system-x86   # once
make run                                   # qemu -kernel, serial in terminal
```

Boot on real hardware / USB (needs `xorriso` too):

```sh
sudo pacman -S --needed grub xorriso mtools
make iso        # -> jupiteros.iso  (then dd to a USB stick, or `make run-iso`)
```

> NOTE: I could not run QEMU in the environment where this was built (no QEMU
> installed, no passwordless sudo), so the graphical output has not been
> visually confirmed — only that it compiles to a valid Multiboot framebuffer
> image. Install QEMU and `make run` to see it; report back if anything is off.

## Roadmap (rest of the spec)

| # | Milestone | Spec section |
|---|-----------|--------------|
| 4b | Upgrade heap to buddy + slab | Kernel / alloc |
| 11 | ext2 read/write + ELF loader | Filesystem |
| 12 | RTL8139 driver + ARP/IP/UDP/TCP + DNS + HTTP | Network |
| 13 | Browser (HTTP client + minimal HTML render) | Apps |

### Notes on the spec

- **"Hello, World!" before protected mode**: under Multiboot/GRUB the CPU is
  *already* in 32-bit protected mode when our kernel runs, so a real-mode
  print requires a custom stage-1 boot sector. That's an optional add-on
  (milestone 0); the multiboot path is the standard, more robust route.
- **64-bit long mode** is a later switch; the current tree targets `-m32`
  (`qemu-system-i386`). Moving to long mode means a new GDT, PML4 paging, and
  an `x86_64-elf` toolchain or `-m64` freestanding build.
- **"bash" terminal**: a real GNU bash needs a POSIX userland (fork/exec,
  ELF dynamic loader, libc). The realistic target is a *bash-like* shell
  (milestone 10) running our own built-in commands; full bash is out of scope
  for a hobby kernel without a libc port.
