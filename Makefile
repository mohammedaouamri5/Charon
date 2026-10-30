CXX := i686-elf-g++
AS  := i686-elf-as
QEMU := qemu-system-i386

CXXFLAGS := -std=c++20 -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra
QEMUFLAGS := -m 128M

all: charon.bin

boot.o: src/boot.s
	$(AS) $< -o $@

kernel.o: src/kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

charon.bin: boot.o kernel.o link.ld
	$(CXX) -ffreestanding -nostdlib -T link.ld -o $@ boot.o kernel.o -lgcc

# Boot the raw multiboot kernel directly (fastest loop)
run: charon.bin
	$(QEMU) $(QEMUFLAGS) -kernel $<

# Build a bootable ISO using the GRUB config in iso/
charon.iso: charon.bin iso/boot/grub/grub.cfg
	cp charon.bin iso/boot/charon.bin
	grub-mkrescue -o $@ iso

# Boot the ISO through GRUB (closer to real hardware / a USB stick)
run-iso: charon.iso
	$(QEMU) $(QEMUFLAGS) -cdrom $<

debug: charon.bin
	$(QEMU) $(QEMUFLAGS) -kernel $< -no-reboot -no-shutdown -d int -D qemu.log

clean:
	rm -f *.o charon.bin charon.iso iso/boot/charon.bin qemu.log

.PHONY: all run run-iso debug clean
