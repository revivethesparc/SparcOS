#
# @file    Makefile
# @brief   OS Framework build system
# @details Builds a bootable x86_64 ISO image from assembly and C sources.
#          Uses GRUB (Multiboot) as the bootloader. The final artifact is
#          os-framework.iso, suitable for QEMU or physical hardware.
# @info    The build is entirely freestanding: no libc, no CRT, no red-zone.
# @author  OS Framework Team
# @date    2026
#

CC       = gcc
LD       = ld
CFLAGS   = -ffreestanding -nostdlib -nostartfiles -mno-red-zone -Wall -Wextra \
           -Werror -Isrc/include
LDFLAGS  = -m elf_x86_64 -T linker.ld
QEMU     = /usr/libexec/qemu-kvm

BOOT_SRC = src/boot/boot.S
KERNEL_C = $(wildcard src/kernel/*.c)
OBJ      = $(patsubst src/%.S, build/%.o, $(BOOT_SRC)) \
           $(patsubst src/%.c, build/%.o, $(KERNEL_C))

.PHONY: all run clean

# @brief  Default target: build the bootable ISO
all: os-framework.iso

# @brief  Assemble .S sources into .o files under build/
build/%.o: src/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# @brief  Compile .c sources into .o files under build/
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# @brief  Link all object files into the ELF64 kernel binary
kernel.bin: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

# @brief  Create the ISO staging directory
iso/:
	mkdir -p iso/boot/grub

# @brief  Install kernel binary into ISO staging
iso/boot/kernel.bin: kernel.bin iso/
	cp kernel.bin iso/boot/kernel.bin

# @brief  Generate GRUB config for boot
iso/boot/grub/grub.cfg: iso/
	printf 'menuentry "OS Framework" {\n\tmultiboot /boot/kernel.bin\n}\n' \
	    > iso/boot/grub/grub.cfg

# @brief  Generate the final bootable ISO via grub2-mkrescue
os-framework.iso: iso/boot/kernel.bin iso/boot/grub/grub.cfg
	grub2-mkrescue -o os-framework.iso iso/ 2>/dev/null

# @brief  Launch the OS under QEMU
run: os-framework.iso
	$(QEMU) -cdrom os-framework.iso

# @brief  Remove all build artifacts
clean:
	rm -rf build/ kernel.bin os-framework.iso iso/
