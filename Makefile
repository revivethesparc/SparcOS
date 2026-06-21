CC       = gcc
LD       = ld
CFLAGS   = -ffreestanding -nostdlib -nostartfiles -mno-red-zone -Wall -Wextra -Werror -Isrc/include
LDFLAGS  = -m elf_x86_64 -T linker.ld
QEMU     = /usr/libexec/qemu-kvm

BOOT_SRC = src/boot/boot.S
KERNEL_C = $(wildcard src/kernel/*.c)
OBJ      = $(patsubst src/%.S, build/%.o, $(BOOT_SRC)) \
           $(patsubst src/%.c, build/%.o, $(KERNEL_C))

.PHONY: all run clean

all: os-framework.iso

build/%.o: src/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

kernel.bin: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

iso/:
	mkdir -p iso/boot/grub

iso/boot/kernel.bin: kernel.bin iso/
	cp kernel.bin iso/boot/kernel.bin

iso/boot/grub/grub.cfg: iso/
	printf 'menuentry "OS Framework" {\n\tmultiboot /boot/kernel.bin\n}\n' > iso/boot/grub/grub.cfg

os-framework.iso: iso/boot/kernel.bin iso/boot/grub/grub.cfg
	grub2-mkrescue -o os-framework.iso iso/ 2>/dev/null

run: os-framework.iso
	$(QEMU) -cdrom os-framework.iso

clean:
	rm -rf build/ kernel.bin os-framework.iso iso/
