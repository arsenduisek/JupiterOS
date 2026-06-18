# JupiterOS build — 32-bit Multiboot kernel, host gcc (-m32), no cross-compiler needed.

CC      := gcc
CFLAGS  := -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-builtin \
           -nostdlib -Wall -Wextra -Iinclude -O2 -std=gnu11
ASFLAGS := -m32
LD      := ld
LDFLAGS := -m elf_i386 -T linker.ld -nostdlib

CSRC := $(wildcard src/*.c)
SSRC := $(wildcard src/*.S)
OBJ  := $(CSRC:.c=.o) $(SSRC:.S=.o)

KERNEL := jupiteros.elf

.PHONY: all run debug clean check iso run-iso

all: $(KERNEL)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/%.o: src/%.S
	$(CC) $(ASFLAGS) -c $< -o $@

$(KERNEL): $(OBJ) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

# Verify the binary is a valid Multiboot image
check: $(KERNEL)
	grub-file --is-x86-multiboot $(KERNEL) && echo "Multiboot: OK"

# Boot directly (no ISO/GRUB needed). Serial is mirrored to your terminal.
run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -serial stdio -vga std -m 256M

# Build a bootable GRUB ISO (for real hardware / USB). Needs grub + xorriso.
iso: $(KERNEL) grub.cfg
	mkdir -p isodir/boot/grub
	cp $(KERNEL) isodir/boot/jupiteros.elf
	cp grub.cfg  isodir/boot/grub/grub.cfg
	grub-mkrescue -o jupiteros.iso isodir
	@echo "Wrote jupiteros.iso"

run-iso: jupiteros.iso
	qemu-system-i386 -cdrom jupiteros.iso -vga std -m 256M

# Boot under GDB stub: in another terminal run  gdb jupiteros.elf  then  target remote :1234
debug: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -serial stdio -s -S

clean:
	rm -f src/*.o $(KERNEL) jupiteros.iso
	rm -rf isodir
