IMAGE_NAME := bleed-kernel
OBJDIR := bin/obj
KERNEL_BIN := bin/bleed-kernel
CC := cc
LD := ld
OBJCOPY := objcopy

MEMSZ = 16G

CFLAGS := -g -O2 -Wall -Wpedantic -Werror -Wextra -std=gnu11 \
          -nostdinc -ffreestanding -fno-stack-protector \
          -fno-stack-check -fno-lto -fno-PIC -fno-pie \
          -ffunction-sections -fdata-sections -fno-omit-frame-pointer \
          -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx -mno-sse2 -mno-red-zone \
          -mcmodel=kernel -I kernel/include -I klibc/include

LDFLAGS := -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 --gc-sections \
           -T kernel.lds

KERNEL_C := $(shell find kernel -name '*.c')
KERNEL_S := $(shell find kernel -name '*.S')

KLIBC_C := $(shell find klibc -name '*.c')
KLIBC_S := $(shell find klibc -name '*.S')

KERNEL_OBJ := $(patsubst %.c,$(OBJDIR)/%.o,$(KERNEL_C)) \
              $(patsubst %.S,$(OBJDIR)/%.o,$(KERNEL_S))
KLIBC_OBJ := $(patsubst %.c,$(OBJDIR)/%.o,$(KLIBC_C)) \
             $(patsubst %.S,$(OBJDIR)/%.o,$(KLIBC_S))
OBJ := $(KERNEL_OBJ) $(KLIBC_OBJ)

.PHONY: all
all: $(IMAGE_NAME).iso

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

limine/limine:
	rm -rf limine
	git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1
	$(MAKE) -C limine

edk2-ovmf:
	curl -L https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/edk2-ovmf.tar.gz | gunzip | tar -xf -

.PHONY: initrd
initrd:
	tar -cf initrd/initrd.tar initrd/etc/splash.txt initrd/fonts/ttyfont.psf

$(IMAGE_NAME).iso: limine/limine $(KERNEL_BIN) initrd
	rm -rf iso_root
	mkdir -p iso_root/boot
	cp -v $(KERNEL_BIN) iso_root/boot/
	cp -v wallpaper.jpg iso_root/
	mkdir -p iso_root/boot/limine
	cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	mkdir -p iso_root/boot
	cp -v initrd/initrd.tar iso_root/boot/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
	./limine/limine bios-install $(IMAGE_NAME).iso
	rm -rf iso_root

.PHONY: run
run: $(IMAGE_NAME).iso
	qemu-system-x86_64 -cdrom $(IMAGE_NAME).iso -boot d -m $(MEMSZ) -serial stdio -display sdl 

.PHONY: run-uefi
run-uefi: edk2-ovmf $(IMAGE_NAME).iso
	qemu-system-x86_64 \
		-m $(MEMSZ) \
		-drive if=pflash,unit=0,format=raw,file=edk2-ovmf/ovmf-code-x86_64.fd,readonly=on \
		-cdrom $(IMAGE_NAME).iso \
		-boot d \
		-no-shutdown -no-reboot \
		-serial stdio \
		-display sdl \
		$(QEMUFLAGS)

.PHONY: clean
clean:
	rm -rf bin $(IMAGE_NAME).iso iso_root edk2-ovmf
	find kernel klibc -name '*.o' -delete
	find kernel klibc -name '*.d' -delete
	find initrd -name '*.tar' -delete
	
