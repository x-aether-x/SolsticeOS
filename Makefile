CC = g++ -Wno-register
NASM = nasm
OBJCOPY = objcopy

EFI_LDS = $(shell pkg-config --variable=libdir gnu-efi)/../lib/elf_x86_64_efi.lds

CFLAGS = -ffreestanding -m64 -g -Wall -Wextra -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-pic -fno-pie -fno-stack-protector -Isrc/include
EFI_CFLAGS = -I$(EFI_INC) -I$(EFI_INC)/x86_64 -I$(EFI_INC)/protocol -fPIC -fshort-wchar -mno-red-zone -mcmodel=large -fno-stack-protector

BOOT_CFLAGS = -Isrc/include -fno-stack-protector -fshort-wchar -mno-red-zone -ffreestanding

FONT_SRC = src/include/FreeSans.sfn
FONT_RAW = build/FreeSans.sfn
FONT_OBJ = build/font.o

OBJS = build/kernel_entry.o build/kernel.o build/printf.o build/console.o build/gdtc.o build/gdts.o build/utils.o build/idtc.o build/idts.o build/ext2.o build/timer.o build/memory.o $(FONT_OBJ)

all: prepare build/kernel.bin build/BOOTX64.EFI

prepare:
	mkdir -p build

build/kernel_entry.o: src/kernel/kernel_entry.asm
	$(NASM) -f elf64 $< -o $@

build/memory.o: src/kernel/memory/memory.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.o: src/kernel/kernel.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/printf.o: src/kernel/misc/printf.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/ext2.o: src/kernel/filesystem/ext2.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/timer.o: src/kernel/idt/timer.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/console.o: src/kernel/misc/console.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/gdtc.o: src/kernel/gdt/gdt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/gdts.o: src/kernel/gdt/gdt.s
	$(NASM) -f elf64 $< -o $@

build/utils.o: src/kernel/utils/utils.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/idtc.o: src/kernel/idt/idt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

build/idts.o: src/kernel/idt/idt.s
	$(NASM) -f elf64 $< -o $@

$(FONT_OBJ): $(FONT_SRC)
	gzip -dc $< > $(FONT_RAW)
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $(FONT_RAW) $@

build/complete_kernel.elf: $(OBJS)
	g++ -T src/kernel/linker.ld -ffreestanding -m64 -nostdlib -no-pie -o $@ $^

build/kernel.bin: build/complete_kernel.elf
	$(OBJCOPY) -O binary $< $@

build/BOOTX64.EFI: src/bootloader/bootloader.cpp
	g++ -Wno-register -Isrc/include -fno-stack-protector -fshort-wchar -mno-red-zone -fPIC -fno-ident -fno-asynchronous-unwind-tables -ffreestanding -c src/bootloader/bootloader.cpp -o build/bootloader.o
	ld -m i386pep --subsystem 10 -shared -Bsymbolic -e efi_main -s -o $@ build/bootloader.o

setup_iso: all
	mkdir -p build/iso/EFI/BOOT
	cp build/BOOTX64.EFI build/iso/EFI/BOOT/
	cp build/kernel.bin build/iso/

# creates a 64MB Fat32 boot image
disk.img: build/BOOTX64.EFI build/kernel.bin
	dd if=/dev/zero of=disk.img bs=1M count=64
	echo -e "o\nn\np\n1\n\n\nt\nef\na\nw" | fdisk disk.img
	sudo losetup -P /dev/loop0 disk.img
	sudo mkfs.vfat -F 32 /dev/loop0p1
	sudo mkdir -p /mnt/tmp
	sudo mount /dev/loop0p1 /mnt/tmp
	sudo mkdir -p /mnt/tmp/EFI/BOOT
	sudo cp build/BOOTX64.EFI /mnt/tmp/EFI/BOOT/
	sudo cp build/kernel.bin /mnt/tmp/
	sudo umount /mnt/tmp
	sudo losetup -d /dev/loop0

build_iso: all
	mkdir -p build/iso/EFI/BOOT
	cp build/BOOTX64.EFI build/iso/EFI/BOOT/
	cp build/kernel.bin build/iso/
	xorriso -as mkisofs -R -f -e EFI/BOOT/BOOTX64.EFI -no-emul-boot -o build/SolsticeOS.iso build/iso

#create a 10MB ext2 test image for filesystem
ext2.img:
	dd if=/dev/zero of=ext2.img bs=1M count=10
	mkfs.ext2 -b 1024 -I 128 ext2.img

run: disk.img ext2.img
	qemu-system-x86_64 -bios /usr/share/edk2/x64/OVMF.4m.fd \
	-drive id=drive0,file=disk.img,format=raw,if=ide \
	-drive id=drive1,file=ext2.img,format=raw,if=ide \
	-serial stdio

debug: setup_iso
	qemu-system-x86_64 -bios /usr/share/edk2/x64/OVMF.4m.fd -drive file=fat:rw:build/iso,format=raw,media=disk -serial stdio -s -S -debugcon file:debug.log -global isa-debugcon.iobase=0x402

clean:
	rm -rf build disk.img ext2.img