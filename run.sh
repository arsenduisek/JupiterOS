#!/usr/bin/env bash
# One-command launcher for JupiterOS.
# Installs QEMU if missing (Arch/CachyOS), builds, and boots the kernel.
set -e

cd "$(dirname "$0")"

if ! command -v qemu-system-i386 >/dev/null 2>&1; then
    echo ">> QEMU not found. Installing qemu-system-x86 (needs sudo password)..."
    if command -v pacman >/dev/null 2>&1; then
        sudo pacman -S --needed --noconfirm qemu-system-x86
    else
        echo "!! Not an Arch/pacman system. Install QEMU manually, then re-run." >&2
        exit 1
    fi
fi

echo ">> Building..."
make

echo ">> Booting JupiterOS in QEMU (serial log mirrored below; close the QEMU window to quit)."
exec qemu-system-i386 -kernel jupiteros.elf -serial stdio -vga std -m 256M
